package com.redsafetw.user_service.service;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.redsafetw.user_service.dto.EdgeCommandRequest;
import com.redsafetw.user_service.dto.EdgeCommandResponse;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.http.HttpStatus;
import org.springframework.messaging.MessagingException;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

import java.time.Duration;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;

@Service
@Transactional
@RequiredArgsConstructor
public class EdgeCommandService {
    private static final Logger logger = LoggerFactory.getLogger(EdgeCommandService.class);
    private static final Duration COMMAND_CACHE_TTL = Duration.ofSeconds(60);
    private static final long SSE_POLL_INTERVAL_MS = 500L;
    private static final long SSE_WAIT_TIMEOUT_MS = 31_000L;

    private final UserEdgeBindRepository userEdgeBindRepository;
    private final EdgeMqttPublisher edgeMqttPublisher;
    private final ObjectMapper objectMapper;
    private final StringRedisTemplate redisTemplate;
    private final EdgeCommandResponseListener edgeCommandResponseListener;

    public EdgeCommandResponse sendCommand(EdgeCommandRequest request, String accessToken) {
        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("sendCommand: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        String edgeId = request.getEdgeId();
        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edgeId)) {
            logger.info("sendCommand: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定", userId, edgeId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        String traceId = UUID.randomUUID().toString();
        String payloadJson = serializePayload(request);

        cacheCommandRequest(traceId, edgeId, request.getCode(), request.getPayload());

        try {
            edgeMqttPublisher.publishEdgeCommand(edgeId, traceId, request.getCode(), payloadJson);
        } catch (MessagingException ex) {
            logger.error("sendCommand: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} MQTT publish failed", userId, edgeId, ex);
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE, "MQTT_PUBLISH_FAILED");
        } catch (Exception ex) {
            logger.error("sendCommand: Unexpected error publishing MQTT command for edge {}", edgeId, ex);
            throw new ResponseStatusException(HttpStatus.INTERNAL_SERVER_ERROR, "MQTT_PUBLISH_FAILED");
        }

        edgeCommandResponseListener.listenForResponse(edgeId, traceId);

        return EdgeCommandResponse.builder()
                .traceId(traceId)
                .build();
    }

    public SseEmitter streamCommandResponse(String traceId, String accessToken) {
        String requestCache = redisTemplate.opsForValue().get(requestKey(traceId));
        if (requestCache == null) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "155");
        }

        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("streamCommandResponse: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        String edgeId = extractEdgeId(requestCache, traceId);
        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edgeId)) {
            logger.info("streamCommandResponse: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定", userId, edgeId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        SseEmitter emitter = new SseEmitter(0L);
        CompletableFuture.runAsync(() -> emitResponse(traceId, emitter));
        return emitter;
    }

    private String serializePayload(EdgeCommandRequest request) {
        if (request.getPayload() == null) {
            return null;
        }
        try {
            return objectMapper.writeValueAsString(request.getPayload());
        } catch (JsonProcessingException ex) {
            logger.warn("sendCommand: Failed to serialize payload for edge {}", request.getEdgeId(), ex);
            return request.getPayload().toString();
        }
    }

    private void cacheCommandRequest(String traceId, String edgeId, String code, JsonNode payload) {
        ObjectNode node = objectMapper.createObjectNode();
        node.put("edge_id", edgeId);
        node.put("code", code);
        if (payload != null) {
            node.set("payload", payload);
        }
        try {
            redisTemplate.opsForValue().set(requestKey(traceId), objectMapper.writeValueAsString(node), COMMAND_CACHE_TTL);
        } catch (Exception ex) {
            logger.error("sendCommand: Failed to cache command request for edge {} trace {}", edgeId, traceId, ex);
            throw new ResponseStatusException(HttpStatus.INTERNAL_SERVER_ERROR, "CACHE_FAILED");
        }
    }

    private String extractEdgeId(String requestCache, String traceId) {
        try {
            JsonNode node = objectMapper.readTree(requestCache);
            String edgeId = node.path("edge_id").asText(null);
            if (edgeId == null || edgeId.isBlank()) {
                logger.warn("streamCommandResponse: Missing edge_id in cached request trace {}", traceId);
                throw new ResponseStatusException(HttpStatus.INTERNAL_SERVER_ERROR, "CACHE_INVALID");
            }
            return edgeId;
        } catch (JsonProcessingException ex) {
            logger.error("streamCommandResponse: Failed to parse cached request for trace {}", traceId, ex);
            throw new ResponseStatusException(HttpStatus.INTERNAL_SERVER_ERROR, "CACHE_INVALID");
        }
    }

    private void emitResponse(String traceId, SseEmitter emitter) {
        try {
            String responseJson = awaitResponse(traceId);
            JsonNode responseNode = objectMapper.readTree(responseJson);
            JsonNode payloadNode = responseNode.path("payload");

            Object data;
            if (payloadNode.isMissingNode() || payloadNode.isNull()) {
                data = "notfound";
            } else if (payloadNode.isTextual()) {
                data = payloadNode.asText();
            } else {
                data = payloadNode;
            }

            emitter.send(SseEmitter.event().name("message").data(data));
            emitter.complete();
        } catch (Exception ex) {
            logger.error("streamCommandResponse: Failed to emit response for trace {}", traceId, ex);
            emitter.completeWithError(ex);
        }
    }

    private String awaitResponse(String traceId) {
        String key = responseKey(traceId);
        long waited = 0L;
        String value = redisTemplate.opsForValue().get(key);
        while (value == null && waited < SSE_WAIT_TIMEOUT_MS) {
            try {
                Thread.sleep(SSE_POLL_INTERVAL_MS);
            } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
                break;
            }
            waited += SSE_POLL_INTERVAL_MS;
            value = redisTemplate.opsForValue().get(key);
        }

        if (value == null) {
            ObjectNode node = objectMapper.createObjectNode();
            node.put("payload", "notfound");
            value = node.toString();
            redisTemplate.opsForValue().set(key, value, COMMAND_CACHE_TTL);
        }
        return value;
    }

    private String requestKey(String traceId) {
        return "cmd:req:" + traceId;
    }

    private String responseKey(String traceId) {
        return "cmd:res:" + traceId;
    }
}
