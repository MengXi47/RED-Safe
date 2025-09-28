package com.redsafetw.mqtt_service.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.redsafetw.mqtt_service.config.MqttProperties;
import com.redsafetw.mqtt_service.service.EdgeMqttPublisher;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
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
import java.time.OffsetDateTime;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

@Service
@RequiredArgsConstructor
public class EdgeHeartbeatManager {
    private static final Logger log = LoggerFactory.getLogger(EdgeHeartbeatManager.class);
    private static final Duration HEARTBEAT_TTL = Duration.ofSeconds(15);
    private static final Duration HEARTBEAT_TIMEOUT = Duration.ofSeconds(10);
    private static final Duration COMMAND_PING_INTERVAL = Duration.ofSeconds(30);
    private static final Pattern EDGE_ID_PATTERN = Pattern.compile("^RED-[0-9A-F]{8}$");

    private final MqttProperties mqttProperties;
    private final StringRedisTemplate redisTemplate;
    private final ObjectMapper objectMapper;
    private final EdgeMqttPublisher edgeMqttPublisher;

    private final Set<String> subscribedTopics = ConcurrentHashMap.newKeySet();
    private final ConcurrentHashMap<String, ScheduledFuture<?>> heartbeatWatchdogs = new ConcurrentHashMap<>();
    private final Map<String, ScheduledFuture<?>> pingTasks = new ConcurrentHashMap<>();
    private final ScheduledExecutorService watchdogScheduler = Executors.newSingleThreadScheduledExecutor(r -> {
        Thread thread = new Thread(r, "edge-heartbeat-watchdog");
        thread.setDaemon(true);
        return thread;
    });
    private MqttAsyncClient client;

    @PostConstruct
    public void start() throws MqttException {
        connectIfNecessary();
    }

    @PreDestroy
    public void shutdown() {
        if (client != null) {
            try {
                client.disconnect();
                client.close();
            } catch (MqttException ex) {
                log.warn("Failed to shut down MQTT heartbeat client cleanly", ex);
            }
        }
        heartbeatWatchdogs.values().forEach(future -> future.cancel(false));
        pingTasks.values().forEach(future -> future.cancel(false));
        watchdogScheduler.shutdownNow();
    }

    public void registerEdge(String edgeId) throws MqttException {
        connectIfNecessary();
        validateEdgeId(edgeId);
        String statusTopic = statusTopic(edgeId);
        if (subscribedTopics.add(statusTopic)) {
            client.subscribe(statusTopic, mqttProperties.getQos()).waitForCompletion();
            log.info("Subscribed to heartbeat topic {}", statusTopic);
        }
        String dataTopic = dataTopic(edgeId);
        if (subscribedTopics.add(dataTopic)) {
            client.subscribe(dataTopic, mqttProperties.getQos()).waitForCompletion();
            log.info("Subscribed to data topic {}", dataTopic);
        }
        sendPing(edgeId);
        armHeartbeatWatchdog(edgeId);
        ensurePingScheduled(edgeId);
    }

    private void validateEdgeId(String edgeId) {
        if (!EDGE_ID_PATTERN.matcher(edgeId).matches()) {
            throw new IllegalArgumentException("edge_id must match pattern RED-[0-9A-F]{8}");
        }
    }

    private synchronized void connectIfNecessary() throws MqttException {
        if (client != null && client.isConnected()) {
            return;
        }
        if (client == null) {
            client = new MqttAsyncClient(mqttProperties.getUri(),
                    "mqtt-service-heartbeat-" + UUID.randomUUID());
            client.setCallback(new HeartbeatCallback());
        }
        if (!client.isConnected()) {
            client.connect(buildOptions()).waitForCompletion();
            log.info("Connected MQTT heartbeat client to {}", mqttProperties.getUri());
        }
    }

    private MqttConnectionOptions buildOptions() {
        MqttConnectionOptions options = new MqttConnectionOptions();
        options.setServerURIs(new String[]{mqttProperties.getUri()});
        options.setAutomaticReconnect(true);
        options.setCleanStart(true);
        options.setSessionExpiryInterval(0L);
        options.setKeepAliveInterval(30);
        options.setUserName(mqttProperties.getUsername());
        if (mqttProperties.getPassword() != null) {
            options.setPassword(mqttProperties.getPassword().getBytes(StandardCharsets.UTF_8));
        }
        return options;
    }

    private String statusTopic(String edgeId) {
        return edgeId + "/status";
    }

    private String dataTopic(String edgeId) {
        return edgeId + "/data";
    }

    private final class HeartbeatCallback implements MqttCallback {
        @Override
        public void disconnected(MqttDisconnectResponse disconnectResponse) {
            log.warn("Heartbeat MQTT client disconnected: {}", disconnectResponse.getReasonString());
        }

        @Override
        public void mqttErrorOccurred(MqttException exception) {
            log.error("Heartbeat MQTT client error", exception);
        }

        @Override
        public void messageArrived(String topic, MqttMessage message) {
            try {
                String payload = new String(message.getPayload(), StandardCharsets.UTF_8);
                String edgeId = extractEdgeId(topic);
                if (topic.endsWith("/status")) {
                    storeHeartbeat(edgeId, payload);
                    armHeartbeatWatchdog(edgeId);
                    ensurePingScheduled(edgeId);
                } else if (topic.endsWith("/data")) {
                    // TODO: Persist edge data payload once downstream pipeline is ready.
                    log.info("Edge data received. edge_id={} payload={}", edgeId, payload);
                } else {
                    log.debug("Received message on unexpected topic {} payload={}", topic, payload);
                }
            } catch (Exception ex) {
                log.warn("Failed to handle heartbeat message on topic {}", topic, ex);
            }
        }

        @Override
        public void deliveryComplete(org.eclipse.paho.mqttv5.client.IMqttToken token) {
            // No-op, we do not publish via this client.
        }

        @Override
        public void connectComplete(boolean reconnect, String serverURI) {
            log.info("Heartbeat MQTT connection {} to {}", reconnect ? "reconnected" : "connected", serverURI);
            if (reconnect) {
                subscribedTopics.forEach(topic -> {
                    try {
                        client.subscribe(topic, mqttProperties.getQos());
                        String edgeId = extractEdgeId(topic);
                        if (topic.endsWith("/status")) {
                            armHeartbeatWatchdog(edgeId);
                            ensurePingScheduled(edgeId);
                        }
                    } catch (MqttException ex) {
                        log.error("Failed to resubscribe heartbeat topic {}", topic, ex);
                    }
                });
            }
        }

        @Override
        public void authPacketArrived(int reasonCode, org.eclipse.paho.mqttv5.common.packet.MqttProperties properties) {
            // No-op
        }
    }

    private void storeHeartbeat(String edgeId, String payload) {
        try {
            JsonNode node = objectMapper.readTree(payload);
            ObjectNode objectNode;
            if (node.isObject()) {
                objectNode = (ObjectNode) node;
            } else {
                objectNode = objectMapper.createObjectNode();
                objectNode.put("raw", payload);
            }
            objectNode.put("edge_id", edgeId);
            objectNode.with("meta").put("received_at", OffsetDateTime.now().toString());
            redisTemplate.opsForValue().set(redisKey(edgeId), objectNode.toString(), HEARTBEAT_TTL);
        } catch (Exception ex) {
            log.warn("Invalid heartbeat payload for edge {}: {}", edgeId, payload, ex);
            ObjectNode fallback = objectMapper.createObjectNode();
            fallback.put("edge_id", edgeId);
            fallback.put("raw", payload);
            fallback.put("received_at", OffsetDateTime.now().toString());
            redisTemplate.opsForValue().set(redisKey(edgeId), fallback.toString(), HEARTBEAT_TTL);
        }
    }

    private String redisKey(String edgeId) {
        return "edge:heartbeat:" + edgeId;
    }

    private String extractEdgeId(String topic) {
        int slashIndex = topic.indexOf('/');
        if (slashIndex > 0) {
            return topic.substring(0, slashIndex);
        }
        return topic;
    }

    private void armHeartbeatWatchdog(String edgeId) {
        ScheduledFuture<?> existing = heartbeatWatchdogs.remove(edgeId);
        if (existing != null) {
            existing.cancel(false);
        }

        ScheduledFuture<?> future = watchdogScheduler.schedule(() -> handleHeartbeatTimeout(edgeId),
                HEARTBEAT_TIMEOUT.toMillis(), java.util.concurrent.TimeUnit.MILLISECONDS);
        heartbeatWatchdogs.put(edgeId, future);
    }

    private void handleHeartbeatTimeout(String edgeId) {
        String statusTopic = statusTopic(edgeId);
        heartbeatWatchdogs.remove(edgeId);
        if (subscribedTopics.remove(statusTopic)) {
            log.warn("Heartbeat timeout for edge {}, unsubscribing topic {}", edgeId, statusTopic);
            try {
                if (client != null && client.isConnected()) {
                    client.unsubscribe(statusTopic).waitForCompletion();
                }
            } catch (MqttException ex) {
                log.warn("Failed to unsubscribe heartbeat topic {} after timeout", statusTopic, ex);
            }
        }

        String dataTopic = dataTopic(edgeId);
        if (subscribedTopics.remove(dataTopic)) {
            try {
                if (client != null && client.isConnected()) {
                    client.unsubscribe(dataTopic).waitForCompletion();
                }
            } catch (MqttException ex) {
                log.warn("Failed to unsubscribe data topic {} after timeout", dataTopic, ex);
            }
        }

        cancelPing(edgeId);
    }

    private void ensurePingScheduled(String edgeId) {
        pingTasks.computeIfAbsent(edgeId, id -> watchdogScheduler.scheduleAtFixedRate(
                () -> sendPing(id),
                COMMAND_PING_INTERVAL.getSeconds(),
                COMMAND_PING_INTERVAL.getSeconds(),
                TimeUnit.SECONDS
        ));
    }

    private void cancelPing(String edgeId) {
        ScheduledFuture<?> existing = pingTasks.remove(edgeId);
        if (existing != null) {
            existing.cancel(false);
        }
    }

    private void sendPing(String edgeId) {
        try {
            String traceId = UUID.randomUUID().toString();
            edgeMqttPublisher.publishEdgeCommand(edgeId, traceId, "100", null);
            log.debug("Dispatched heartbeat command code=100 trace_id={} to edge {}", traceId, edgeId);
        } catch (Exception ex) {
            log.warn("Failed to dispatch heartbeat command to edge {}", edgeId, ex);
        }
    }
}
