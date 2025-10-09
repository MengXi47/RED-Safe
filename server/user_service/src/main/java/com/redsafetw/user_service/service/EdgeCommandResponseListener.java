package com.redsafetw.user_service.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.redsafetw.user_service.config.MqttProperties;
import lombok.RequiredArgsConstructor;
import org.eclipse.paho.mqttv5.client.MqttAsyncClient;
import org.eclipse.paho.mqttv5.client.MqttCallback;
import org.eclipse.paho.mqttv5.client.MqttConnectionOptions;
import org.eclipse.paho.mqttv5.client.MqttDisconnectResponse;
import org.eclipse.paho.mqttv5.common.MqttException;
import org.eclipse.paho.mqttv5.common.MqttMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@Service
@RequiredArgsConstructor
public class EdgeCommandResponseListener {

    private static final Logger logger = LoggerFactory.getLogger(EdgeCommandResponseListener.class);
    private static final Duration RESPONSE_CACHE_TTL = Duration.ofSeconds(60);
    private static final Duration RESPONSE_TIMEOUT = Duration.ofSeconds(30);

    private final MqttProperties mqttProperties;
    private final ObjectMapper objectMapper;
    private final StringRedisTemplate redisTemplate;

    // 針對指定 edge 與 traceId 非同步啟動監聽程序
    public void listenForResponse(String edgeId, String traceId) {
        CompletableFuture.runAsync(() -> awaitAndCache(edgeId, traceId));
    }

    private void awaitAndCache(String edgeId, String traceId) {
        JsonNode payloadNode = null;
        try {
            payloadNode = awaitPayload(edgeId, traceId).orElse(null);
        } catch (Exception ex) {
            logger.error("listenForResponse: Unexpected error when awaiting MQTT response for edge {} trace {}", edgeId, traceId, ex);
        }

        ObjectNode cacheNode = objectMapper.createObjectNode();
        if (payloadNode != null) {
            // 成功取得資料時把完整 payload 寫入 Redis
            cacheNode.set("payload", payloadNode);
        } else {
            // 未在時限內取得資料時寫入 notfound
            cacheNode.put("payload", "notfound");
        }
        try {
            String key = responseKey(traceId);
            redisTemplate.opsForValue().set(key, objectMapper.writeValueAsString(cacheNode), RESPONSE_CACHE_TTL);
        } catch (Exception ex) {
            logger.error("listenForResponse: Failed to cache response for trace {}", traceId, ex);
        }
    }

    private Optional<JsonNode> awaitPayload(String edgeId, String traceId) {
        String topic = edgeId + "/data";
        String clientId = "user-service-listener-" + UUID.randomUUID();
        MqttAsyncClient client = null;
        try {
            CountDownLatch latch = new CountDownLatch(1);
            final JsonNode[] resultHolder = new JsonNode[1];

            client = new MqttAsyncClient(mqttProperties.getUri(), clientId);
            client.setCallback(buildCallback(topic, traceId, resultHolder, latch));

            MqttConnectionOptions options = buildOptions();
            client.connect(options).waitForCompletion();

            // 直接呼叫與 heartbeat_service 相同的訂閱方式，讓 Paho 內部自行管理 listener 清單，避免出現空陣列問題
            client.subscribe(topic, mqttProperties.getQos()).waitForCompletion();

            boolean received = latch.await(RESPONSE_TIMEOUT.toMillis(), TimeUnit.MILLISECONDS);

            if (received) {
                return Optional.ofNullable(resultHolder[0]);
            }
            return Optional.empty();
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
            logger.warn("listenForResponse: Interrupted while waiting for MQTT response for edge {} trace {}", edgeId, traceId, ex);
            return Optional.empty();
        } catch (MqttException ex) {
            logger.error("listenForResponse: MQTT error for edge {} trace {}", edgeId, traceId, ex);
            return Optional.empty();
        } finally {
            if (client != null) {
                try {
                    client.unsubscribe(topic);
                } catch (MqttException e) {
                    logger.debug("listenForResponse: Failed to unsubscribe MQTT client for trace {}", traceId, e);
                }
                try {
                    if (client.isConnected()) {
                        client.disconnect().waitForCompletion();
                    }
                } catch (MqttException e) {
                    logger.debug("listenForResponse: Failed to disconnect MQTT client for trace {}", traceId, e);
                }
                try {
                    client.close();
                } catch (MqttException e) {
                    logger.debug("listenForResponse: Failed to close MQTT client for trace {}", traceId, e);
                }
            }
        }
    }

    private MqttCallback buildCallback(String expectedTopic,
                                       String traceId,
                                       JsonNode[] resultHolder,
                                       CountDownLatch latch) {
        return new MqttCallback() {
            @Override
            public void disconnected(MqttDisconnectResponse disconnectResponse) {
                logger.debug("listenForResponse: MQTT client disconnected while awaiting trace {}: {}", traceId,
                        disconnectResponse.getReasonString());
            }

            @Override
            public void mqttErrorOccurred(MqttException exception) {
                logger.error("listenForResponse: MQTT client error while awaiting trace {}", traceId, exception);
                latch.countDown();
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) {
                if (!expectedTopic.equals(topic)) {
                    return;
                }
                handleMessage(traceId, message, resultHolder, latch);
            }

            @Override
            public void deliveryComplete(org.eclipse.paho.mqttv5.client.IMqttToken token) {
                // 此 client 僅用於訂閱，不會發送訊息
            }

            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                logger.debug("listenForResponse: MQTT client {} connection to {} for trace {}", reconnect ? "reconnected" : "established",
                        serverURI, traceId);
            }

            @Override
            public void authPacketArrived(int reasonCode, org.eclipse.paho.mqttv5.common.packet.MqttProperties properties) {
                // 不使用 MQTT v5 auth 延伸機制
            }
        };
    }

    private void handleMessage(String traceId, MqttMessage message, JsonNode[] resultHolder, CountDownLatch latch) {
        try {
            // MQTT payload 預期為 JSON 格式，需解析出內層 payload.trace_id
            String payload = new String(message.getPayload(), StandardCharsets.UTF_8);
            JsonNode root = objectMapper.readTree(payload);
            JsonNode payloadNode = root.path("payload");
            String responseTraceId = payloadNode.path("trace_id").asText(null);
            if (traceId.equals(responseTraceId)) {
                resultHolder[0] = payloadNode;
                latch.countDown();
            }
        } catch (Exception ex) {
            logger.warn("listenForResponse: Failed to parse MQTT message for trace {}", traceId, ex);
        }
    }

    private MqttConnectionOptions buildOptions() {
        // 建立 MQTT 連線設定，使用設定檔中的連線資訊
        MqttConnectionOptions options = new MqttConnectionOptions();
        options.setServerURIs(new String[]{mqttProperties.getUri()});
        options.setAutomaticReconnect(true);
        options.setCleanStart(true);
        options.setSessionExpiryInterval(0L);
        options.setConnectionTimeout(10);
        options.setKeepAliveInterval(30);
        options.setUserName(mqttProperties.getUsername());
        if (mqttProperties.getPassword() != null) {
            options.setPassword(mqttProperties.getPassword().getBytes(StandardCharsets.UTF_8));
        }
        return options;
    }

    private String responseKey(String traceId) {
        return "cmd:res:" + traceId;
    }
}
