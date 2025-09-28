package com.redsafetw.mqtt_service.service;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.redsafetw.mqtt_service.config.MqttIntegrationConfig.MqttCommandGateway;
import com.redsafetw.mqtt_service.config.MqttProperties;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

/**
 * 發佈 Edge 指令的核心服務：會包裝 trace_id / code / payload 成 JSON 字串。
 */
@Service
@RequiredArgsConstructor
public class EdgeMqttPublisher {
    private static final Logger log = LoggerFactory.getLogger(EdgeMqttPublisher.class);

    private final MqttCommandGateway mqttGateway;
    private final MqttProperties mqttProperties;
    private final ObjectMapper objectMapper;

    /**
     * 發布邊緣裝置指令。會包裝 trace_id 與 payload 形成 JSON 後送往 Edge。
     */
    public void publishEdgeCommand(String edgeId, String traceId, String code, String payloadJson) {
        ObjectNode message = objectMapper.createObjectNode();
        message.put("trace_id", traceId);
        message.put("code", code);

        if (payloadJson != null && !payloadJson.isBlank()) {
            try {
                JsonNode payloadNode = objectMapper.readTree(payloadJson);
                message.set("payload", payloadNode);
            } catch (JsonProcessingException ex) {
                log.warn("Invalid payload JSON provided for edge command. edgeId={} traceId={}", edgeId, traceId, ex);
                message.put("payload", payloadJson);
            }
        }

        String topic = topicForEdge(edgeId);
        mqttGateway.publish(topic, message.toString());
        log.info("Published MQTT command. topic={} traceId={} qos={}", topic, traceId, mqttProperties.getQos());
    }

    private String topicForEdge(String edgeId) {
        return edgeId + "/cmd";
    }
}
