package com.redsafetw.user_service.service;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.stereotype.Service;

import java.time.Duration;
import java.util.Optional;

/**
 * Edge 指令請求與回應的 Redis 存取服務。
 */
@Service
@RequiredArgsConstructor
public class EdgeCommandRedisService {

    private static final Logger log = LoggerFactory.getLogger(EdgeCommandRedisService.class);
    private static final Duration DEFAULT_TTL = Duration.ofSeconds(60);

    private final StringRedisTemplate redisTemplate;
    private final ObjectMapper objectMapper;

    /**
     * 將指令請求內容寫入 Redis，包含 traceId/edgeId/code/payload。
     */
    public void storeRequest(String traceId, String edgeId, String code, JsonNode payload) {
        ObjectNode node = objectMapper.createObjectNode();
        node.put("trace_id", traceId);
        node.put("edge_id", edgeId);
        node.put("code", code);
        if (payload != null) {
            node.set("payload", payload);
        }
        putJson(buildRequestKey(traceId), node);
    }

    /**
     * 依 traceId 讀取 Redis 中的指令請求 JSON。
     */
    public Optional<JsonNode> getRequest(String traceId) {
        return getJson(buildRequestKey(traceId));
    }

    /**
     * 將 Edge 回應內容寫入 Redis，若 payload 為 null 則寫入 notfound。
     */
    public void storeResponse(String traceId, JsonNode responsePayload) {
        if (responsePayload == null) {
            storeResponseRaw(traceId, "notfound");
            return;
        }
        putJson(buildResponseKey(traceId), responsePayload);
    }

    /**
     * 將 Edge 回應內容以純字串形式寫入 Redis。
     */
    public void storeResponseRaw(String traceId, String payload) {
        redisTemplate.opsForValue().set(buildResponseKey(traceId), payload, DEFAULT_TTL);
    }

    /**
     * 依 traceId 讀取 JSON 形式的指令回應內容。
     */
    public Optional<JsonNode> getResponse(String traceId) {
        Optional<String> raw = Optional.ofNullable(redisTemplate.opsForValue().get(buildResponseKey(traceId)));
        return raw.flatMap(this::readJsonValue);
    }

    /**
     * 依 traceId 讀取純字串形式的指令回應內容。
     */
    public Optional<String> getResponseRaw(String traceId) {
        return Optional.ofNullable(redisTemplate.opsForValue().get(buildResponseKey(traceId)));
    }

    /**
     * 共用工具：將 JsonNode 寫入 Redis。
     */
    private void putJson(String key, JsonNode value) {
        try {
            redisTemplate.opsForValue().set(key, objectMapper.writeValueAsString(value), DEFAULT_TTL);
        } catch (JsonProcessingException e) {
            log.warn("Failed to serialize JSON for Redis key {}", key, e);
        }
    }

    /**
     * 共用工具：讀取 Redis 中的 JSON 字串並轉成 JsonNode。
     */
    private Optional<JsonNode> getJson(String key) {
        String value = redisTemplate.opsForValue().get(key);
        if (value == null) {
            return Optional.empty();
        }
        return readJsonValue(value);
    }

    /**
     * 共用工具：解析 JSON 字串成 JsonNode。
     */
    private Optional<JsonNode> readJsonValue(String value) {
        try {
            return Optional.ofNullable(objectMapper.readTree(value));
        } catch (JsonProcessingException e) {
            log.warn("Failed to parse JSON from Redis value {}", value, e);
            return Optional.empty();
        }
    }

    /**
     * 產生指令請求的 Redis key。
     */
    private String buildRequestKey(String traceId) {
        return "cmd:req:" + traceId;
    }

    /**
     * 產生指令回應的 Redis key。
     */
    private String buildResponseKey(String traceId) {
        return "cmd:res:" + traceId;
    }
}
