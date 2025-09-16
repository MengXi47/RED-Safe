package com.redsafetw.ios_service.grpc;

import com.grpc.jwt.JwtServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

/**
 * gRPC 客戶端設定
 */
@Configuration
public class GrpcClientsConfig {

    @Bean
    JwtServiceGrpc.JwtServiceBlockingStub jwtBlockingStub(GrpcChannelFactory channels) {
        return JwtServiceGrpc.newBlockingStub(channels.createChannel("user"));
    }
}
