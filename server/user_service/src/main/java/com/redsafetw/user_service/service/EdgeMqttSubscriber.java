package com.redsafetw.user_service.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.redsafetw.user_service.config.MqttProperties;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import lombok.RequiredArgsConstructor;
import org.eclipse.paho.mqttv5.client.MqttAsyncClient;
import org.eclipse.paho.mqttv5.client.MqttCallback;
import org.eclipse.paho.mqttv5.client.MqttConnectionOptions;
import org.eclipse.paho.mqttv5.client.MqttDisconnectResponse;
import org.eclipse.paho.mqttv5.client.persist.MemoryPersistence;
import org.eclipse.paho.mqttv5.common.MqttException;
import org.eclipse.paho.mqttv5.common.MqttMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * 維持長連線並訂閱 edge data topic。
 */
@Service
@RequiredArgsConstructor
public class EdgeMqttSubscriber {

    private static final Logger log = LoggerFactory.getLogger(EdgeMqttSubscriber.class);

    private final ObjectMapper objectMapper;
    private final MqttProperties mqttProperties;

    private final ConcurrentMap<String, CompletableFuture<JsonNode>> pendingTraceMap = new ConcurrentHashMap<>();
    private final ConcurrentMap<String, AtomicInteger> topicRefCounts = new ConcurrentHashMap<>();
    private final Set<String> subscribedTopics = ConcurrentHashMap.newKeySet();
    private final ScheduledExecutorService timeoutScheduler = Executors.newSingleThreadScheduledExecutor(r -> {
        Thread thread = new Thread(r, "edge-command-mqtt-timer");
        thread.setDaemon(true);
        return thread;
    });

    private volatile MqttAsyncClient client;

    /**
     * 初始化 MQTT 客戶端連線，若建連失敗則延後由第一次訂閱時再重試。
     */
    @PostConstruct
    public void initialize() {
        try {
            connectIfNecessary();
        } catch (MqttException ex) {
            log.warn("Initial MQTT connect failed, will retry lazily", ex);
        }
    }

    /**
     * 訂閱 {edgeId}/data 並等待 traceId 對應的 MQTT 回應。
     */
    public CompletableFuture<JsonNode> awaitResponse(String edgeId, String traceId, Duration timeout) {
        CompletableFuture<JsonNode> future = new CompletableFuture<>();
        String topic = dataTopic(edgeId);
        pendingTraceMap.put(traceId, future);
        try {
            connectIfNecessary();
            subscribeTopic(topic);
        } catch (MqttException ex) {
            log.error("Failed to subscribe topic {} for trace {}", topic, traceId, ex);
            pendingTraceMap.remove(traceId);
            future.completeExceptionally(ex);
            throw new IllegalStateException("MQTT_SUBSCRIBE_FAILED", ex);
        }

        ScheduledFuture<?> timeoutFuture = timeoutScheduler.schedule(() -> {
            if (future.complete(null)) {
                log.debug("Await response timed out. edge_id={} trace_id={}", edgeId, traceId);
            }
        }, timeout.toMillis(), TimeUnit.MILLISECONDS);

        future.whenComplete((payload, throwable) -> {
            timeoutFuture.cancel(false);
            pendingTraceMap.remove(traceId);
            releaseTopic(topic);
        });

        return future;
    }

    /**
     * Bean 銷毀時釋放排程器與 MQTT 資源。
     */
    @PreDestroy
    public void shutdown() {
        timeoutScheduler.shutdownNow();
        if (client != null) {
            try {
                client.disconnectForcibly();
                client.close();
            } catch (MqttException ex) {
                log.warn("Failed to close MQTT subscriber client cleanly", ex);
            }
        }
    }

    /**
     * 確保指定 topic 已訂閱，並累計引用計數。
     */
    private void subscribeTopic(String topic) throws MqttException {
        AtomicInteger counter = topicRefCounts.compute(topic, (key, existing) -> {
            if (existing == null) {
                return new AtomicInteger(1);
            }
            existing.incrementAndGet();
            return existing;
        });

        if (counter.get() == 1) {
            ensureClientConnected();
            client.subscribe(topic, mqttProperties.getQos()).waitForCompletion();
            subscribedTopics.add(topic);
            log.info("Subscribed command listener topic={}", topic);
        }
    }

    /**
     * 單一 trace 完成時遞減 topic 引用計數並視情況解除訂閱。
     */
    private void releaseTopic(String topic) {
        topicRefCounts.computeIfPresent(topic, (key, counter) -> {
            int remaining = counter.decrementAndGet();
            if (remaining <= 0) {
                unsubscribeTopic(topic);
                return null;
            }
            return counter;
        });
    }

    /**
     * 執行實際的 MQTT 解除訂閱並同步更新快取集合。
     */
    private void unsubscribeTopic(String topic) {
        if (client == null || !client.isConnected()) {
            subscribedTopics.remove(topic);
            return;
        }
        try {
            client.unsubscribe(topic).waitForCompletion();
            log.info("Unsubscribed command listener topic={}", topic);
        } catch (MqttException ex) {
            log.warn("Failed to unsubscribe topic {}", topic, ex);
        } finally {
            subscribedTopics.remove(topic);
        }
    }

    /**
     * 建立 MQTT 客戶端並在必要時重新連線。
     */
    private synchronized void connectIfNecessary() throws MqttException {
        if (client != null && client.isConnected()) {
            return;
        }
        if (client == null) {
            String clientId = "user-service-command-" + UUID.randomUUID();
            client = new MqttAsyncClient(mqttProperties.getUri(), clientId, new MemoryPersistence());
            client.setCallback(new CommandCallback());
        }
        ensureClientConnected();
    }

    /**
     * 若客戶端尚未連線則立即呼叫 connect。
     */
    private void ensureClientConnected() throws MqttException {
        if (client != null && !client.isConnected()) {
            client.connect(buildOptions()).waitForCompletion();
            log.info("Connected command MQTT client to {}", mqttProperties.getUri());
        }
    }

    /**
     * 建立預設的 MQTT 連線設定。
     */
    private MqttConnectionOptions buildOptions() {
        return getMqttConnectionOptions(mqttProperties);
    }

    /**
     * 依輸入 MqttProperties 建立 MQTT 連線選項。
     */
    public static MqttConnectionOptions getMqttConnectionOptions(MqttProperties mqttProperties) {
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

    /**
     * 從 JSON 節點擷取 trace_id / traceId。
     */
    private Optional<String> extractTraceId(JsonNode node) {
        if (node == null) {
            return Optional.empty();
        }
        JsonNode traceNode = findTraceNode(node);
        if (traceNode != null && !traceNode.isNull()) {
            return Optional.of(traceNode.asText());
        }
        return Optional.empty();
    }

    /**
     * 在主體或 payload 節點尋找 traceId 欄位。
     */
    private JsonNode findTraceNode(JsonNode node) {
        if (node.hasNonNull("trace_id")) {
            return node.get("trace_id");
        }
        if (node.hasNonNull("traceId")) {
            return node.get("traceId");
        }
        JsonNode payloadNode = node.path("payload");
        if (payloadNode.isObject()) {
            if (payloadNode.hasNonNull("trace_id")) {
                return payloadNode.get("trace_id");
            }
            if (payloadNode.hasNonNull("traceId")) {
                return payloadNode.get("traceId");
            }
        }
        return null;
    }

    /**
     * 產生指定 edge 對應的資料 topic。
     */
    private String dataTopic(String edgeId) {
        return edgeId + "/data";
    }

    private final class CommandCallback implements MqttCallback {

        /**
         * 連線中斷時記錄原因以利追蹤。
         */
        @Override
        public void disconnected(MqttDisconnectResponse disconnectResponse) {
            log.warn("MQTT command client disconnected: {}", disconnectResponse.getReasonString());
        }

        /**
         * MQTT 錯誤事件的統一記錄點。
         */
        @Override
        public void mqttErrorOccurred(MqttException exception) {
            log.error("MQTT command client error", exception);
        }

        /**
         * 接收 {edge_id}/data 訊息並嘗試完成對應的 future。
         */
        @Override
        public void messageArrived(String topic, MqttMessage message) {
            String payload = new String(message.getPayload(), StandardCharsets.UTF_8);
            log.info("Received command MQTT message. topic={} payload={}", topic, payload);
            try {
                JsonNode node = objectMapper.readTree(payload);
                Optional<String> traceId = extractTraceId(node);
                if (traceId.isPresent()) {
                    CompletableFuture<JsonNode> future = pendingTraceMap.get(traceId.get());
                    if (future != null && !future.isDone()) {
                        future.complete(node);
                    } else {
                        log.debug("No pending future matches trace {} for topic {}", traceId.get(), topic);
                    }
                } else {
                    log.debug("MQTT command message without trace_id, ignoring. topic={} payload={}", topic, payload);
                }
            } catch (Exception ex) {
                log.warn("Failed to parse MQTT command payload on topic {}", topic, ex);
            }
        }

        /**
         * 訂閱端無需處理交付完成事件。
         */
        @Override
        public void deliveryComplete(org.eclipse.paho.mqttv5.client.IMqttToken token) {
            // No-op for subscriber
        }

        /**
         * 連線完成或重連後重新訂閱既有 topic。
         */
        @Override
        public void connectComplete(boolean reconnect, String serverURI) {
            log.info("Command MQTT client {} to {}", reconnect ? "reconnected" : "connected", serverURI);
            if (reconnect) {
                subscribedTopics.forEach(topic -> {
                    try {
                        client.subscribe(topic, mqttProperties.getQos()).waitForCompletion();
                        log.info("Resubscribed command topic {}", topic);
                    } catch (MqttException ex) {
                        log.error("Failed to resubscribe topic {}", topic, ex);
                    }
                });
            }
        }

        /**
         * 目前未使用 auth packet 事件，保留空實作。
         */
        @Override
        public void authPacketArrived(int reasonCode, org.eclipse.paho.mqttv5.common.packet.MqttProperties properties) {
            // No-op
        }
    }
}
