package com.redsafetw.bind_service.grpc;

import com.grpc.edge.EdgeServiceGrpc;
import com.grpc.auth.AuthServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

@Configuration
public class GrpcClientsConfig {
    @Bean
    EdgeServiceGrpc.EdgeServiceBlockingStub edgeBlockingStub(GrpcChannelFactory channels) {
        return EdgeServiceGrpc.newBlockingStub(channels.createChannel("edge"));
    }

    @Bean
    AuthServiceGrpc.AuthServiceBlockingStub authBlockingStub(GrpcChannelFactory channels) {
        return AuthServiceGrpc.newBlockingStub(channels.createChannel("auth"));
    }
}
