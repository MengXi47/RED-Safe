package com.redsafetw.edge_service.grpc;

import com.grpc.mqtt.MqttServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

@Configuration
public class GrpcClientsConfig {

    @Bean
    MqttServiceGrpc.MqttServiceBlockingStub mqttBlockingStub(GrpcChannelFactory channels) {
        return MqttServiceGrpc.newBlockingStub(channels.createChannel("mqtt"));
    }
}

