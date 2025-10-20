package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.dto.EdgeIdListResponse;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.io.IOException;

import java.util.List;
import java.util.UUID;

/**
 * 獲取終端裝置服務
 *
 * @create 2025-10-01
 */
@Service
@Transactional
@RequiredArgsConstructor
public class EdgeListService {
    private static final Logger logger = LoggerFactory.getLogger(EdgeListService.class);
    private final UserEdgeBindRepository userEdgeBindRepository;
    private final StringRedisTemplate redisTemplate;
    private final TokenVerifier tokenVerifier;
    private final ObjectMapper objectMapper = new ObjectMapper();

    public EdgeIdListResponse getEdgeIdList(String accessToken) {
        UUID userId = tokenVerifier.requireUserId(accessToken);

        List<UserEdgeBindDomain> edgeBindings = userEdgeBindRepository.findByUserId(userId);
        if (edgeBindings.isEmpty()) {
            logger.info("getEdgeIdList: {\"user_id\":\"{}\"} edge_id 數量為 0", userId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "136");
        }

        List<EdgeIdListResponse.EdgeItem> edges = edgeBindings.stream()
                .map(bind -> EdgeIdListResponse.EdgeItem.builder()
                        .edgeId(bind.getEdgeId())
                        .displayName(bind.getDisplayName())
                        .isOnline(isEdgeOnline(bind.getEdgeId()))
                        .build())
                .toList();

        return EdgeIdListResponse.builder()
                .edges(edges)
                .build();
    }

    private boolean isEdgeOnline(String edgeId) {
        String key = "edge:heartbeat:" + edgeId;
        String value = redisTemplate.opsForValue().get(key);
        if (value == null) {
            return false;
        }
        try {
            JsonNode node = objectMapper.readTree(value);
            String status = node.path("status").asText("");
            return "online".equalsIgnoreCase(status);
        } catch (IOException e) {
            logger.error("Failed to parse heartbeat value for edgeId {}: {}", edgeId, e.getMessage());
            return false;
        }
    }
}
