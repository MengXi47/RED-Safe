package com.redsafetw.user_service.service;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.redsafetw.user_service.dto.EdgeCommandRequest;
import com.redsafetw.user_service.dto.EdgeCommandResponse;
import com.redsafetw.user_service.grpc.MqttGrpcClient;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;

@Service
@Transactional
@RequiredArgsConstructor
public class EdgeCommandService {
    private static final Logger logger = LoggerFactory.getLogger(EdgeCommandService.class);

    private final UserEdgeBindRepository userEdgeBindRepository;
    private final MqttGrpcClient mqttGrpcClient;
    private final ObjectMapper objectMapper;

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

        String errorCode = mqttGrpcClient.publishEdgeCommand(edgeId, traceId, request.getCode(), payloadJson);
        if (!"0".equals(errorCode)) {
            logger.error("sendCommand: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} MQTT error {}", userId, edgeId, errorCode);
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE, errorCode);
        }

        return EdgeCommandResponse.builder()
                .traceId(traceId)
                .build();
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
}
