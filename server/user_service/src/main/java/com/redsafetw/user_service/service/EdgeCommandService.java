package com.redsafetw.user_service.service;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.redsafetw.user_service.dto.EdgeCommandRequest;
import com.redsafetw.user_service.dto.EdgeCommandResponse;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.messaging.MessagingException;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.time.Duration;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;

/**
 * Edge 指令下發服務。
 */
@Service
@Transactional
@RequiredArgsConstructor
public class EdgeCommandService {
    private static final Logger logger = LoggerFactory.getLogger(EdgeCommandService.class);
    private static final Duration RESPONSE_TIMEOUT = Duration.ofSeconds(10);

    private final UserEdgeBindRepository userEdgeBindRepository;
    private final EdgeMqttPublisher edgeMqttPublisher;
    private final EdgeMqttSubscriber edgeMqttSubscriber;
    private final EdgeCommandRedisService edgeCommandRedisService;
    private final ObjectMapper objectMapper;
    private final TokenVerifier tokenVerifier;

    /**
     * 驗證後發送 Edge 指令並回傳 traceId。
     */
    public EdgeCommandResponse sendCommand(EdgeCommandRequest request, String accessToken) {
        UUID userId = tokenVerifier.requireUserId(accessToken);

        String edgeId = request.getEdgeId();
        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edgeId)) {
            logger.info("sendCommand: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定", userId, edgeId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        String traceId = UUID.randomUUID().toString();
        edgeCommandRedisService.storeRequest(traceId, edgeId, request.getCode(), request.getPayload());

        CompletableFuture<JsonNode> responseFuture;
        try {
            responseFuture = edgeMqttSubscriber.awaitResponse(edgeId, traceId, RESPONSE_TIMEOUT);
        } catch (IllegalStateException ex) {
            logger.error("sendCommand: Subscribe MQTT failed for edge {} trace {}", edgeId, traceId, ex);
            edgeCommandRedisService.storeResponseRaw(traceId, "notfound");
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE, "MQTT_SUBSCRIBE_FAILED", ex);
        }
        responseFuture.whenComplete((payload, throwable) -> handleResponse(traceId, payload, throwable));

        String payloadJson = serializePayload(request);

        try {
            edgeMqttPublisher.publishEdgeCommand(edgeId, traceId, request.getCode(), payloadJson);
        } catch (MessagingException ex) {
            logger.error("sendCommand: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} MQTT publish failed", userId, edgeId, ex);
            edgeCommandRedisService.storeResponseRaw(traceId, "notfound");
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE, "MQTT_PUBLISH_FAILED");
        } catch (Exception ex) {
            logger.error("sendCommand: Unexpected error publishing MQTT command for edge {}", edgeId, ex);
            edgeCommandRedisService.storeResponseRaw(traceId, "notfound");
            throw new ResponseStatusException(HttpStatus.INTERNAL_SERVER_ERROR, "MQTT_PUBLISH_FAILED");
        }

        return EdgeCommandResponse.builder()
                .traceId(traceId)
                .build();
    }

    /**
     * MQTT 回應完成時寫入 Redis。
     */
    private void handleResponse(String traceId, JsonNode payload, Throwable throwable) {
        if (throwable != null || payload == null) {
            logger.debug("handleResponse: trace {} notfound payload due to {}", traceId,
                    throwable != null ? throwable.getMessage() : "timeout");
            edgeCommandRedisService.storeResponseRaw(traceId, "notfound");
            return;
        }
        JsonNode payloadNode = extractPayloadNode(payload);
        edgeCommandRedisService.storeResponse(traceId, payloadNode != null ? payloadNode : payload);
    }

    /**
     * 取得內層 payload 結構 (若存在)。
     */
    private JsonNode extractPayloadNode(JsonNode node) {
        if (node == null) {
            return null;
        }
        if (node.has("payload") && node.get("payload").isObject()) {
            return node.get("payload");
        }
        return node;
    }

    /**
     * 將 payload 序列化供 MQTT 發佈。
     */
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
}
