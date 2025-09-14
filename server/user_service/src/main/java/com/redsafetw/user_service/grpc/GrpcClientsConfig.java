package com.redsafetw.user_service.grpc;

import com.grpc.edge.EdgeServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

@Configuration
public class GrpcClientsConfig {

    @Bean
    EdgeServiceGrpc.EdgeServiceBlockingStub edgeBlockingStub(GrpcChannelFactory channels) {
        // "edge" 對應到 application.properties 的命名通道
        return EdgeServiceGrpc.newBlockingStub(channels.createChannel("edge"));
    }
}