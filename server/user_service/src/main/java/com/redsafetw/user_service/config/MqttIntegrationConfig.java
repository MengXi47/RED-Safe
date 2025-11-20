package com.redsafetw.user_service.config;

import lombok.RequiredArgsConstructor;
import org.eclipse.paho.mqttv5.client.MqttConnectionOptions;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.annotation.MessagingGateway;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.mqtt.outbound.Mqttv5PahoMessageHandler;
import org.springframework.integration.mqtt.support.MqttHeaders;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.handler.annotation.Header;
import org.springframework.messaging.handler.annotation.Payload;

import java.nio.charset.StandardCharsets;
import java.util.UUID;

import static com.redsafetw.user_service.service.EdgeMqttSubscriber.getMqttConnectionOptions;

@Configuration
@RequiredArgsConstructor
public class MqttIntegrationConfig {

    private final MqttProperties mqttProperties;

    @Bean
    public MessageChannel mqttOutboundChannel() {
        return new DirectChannel();
    }

    @Bean
    @ServiceActivator(inputChannel = "mqttOutboundChannel")
    public MessageHandler mqttOutboundHandler() {
        String clientId = "user-service-mqtt-" + UUID.randomUUID();
        Mqttv5PahoMessageHandler handler = new Mqttv5PahoMessageHandler(buildOptions(), clientId);
        handler.setAsync(false);
        handler.setDefaultQos(mqttProperties.getQos());
        handler.setDefaultRetained(false);
        return handler;
    }

    private MqttConnectionOptions buildOptions() {
        return getMqttConnectionOptions(mqttProperties);
    }

    @MessagingGateway(defaultRequestChannel = "mqttOutboundChannel")
    public interface MqttCommandGateway {
        void publish(@Header(MqttHeaders.TOPIC) String topic, @Payload String payload);
    }
}
