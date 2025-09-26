package com.redsafetw.mqtt_service.service;

import com.redsafetw.mqtt_service.config.MqttIntegrationConfig.MqttCommandGateway;
import com.redsafetw.mqtt_service.config.MqttProperties;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

/**
 * 實際執行 MQTT 發布的服務
 */
@Service
@RequiredArgsConstructor
public class EdgeMqttPublisher {
    private static final Logger log = LoggerFactory.getLogger(EdgeMqttPublisher.class);

    private final MqttCommandGateway mqttGateway;
    private final MqttProperties mqttProperties;

    /**
     * 發布邊緣裝置指令
     */
    public void publishEdgeCommand(String edgeId, String code) {
        mqttGateway.publish(edgeId, code);
        log.info("Published MQTT message. edgeId={} qos={}", edgeId, mqttProperties.getQos());
    }
}
