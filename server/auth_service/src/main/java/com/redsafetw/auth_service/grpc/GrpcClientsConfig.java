package com.redsafetw.auth_service.grpc;

import com.grpc.notify.NotifyServiceGrpc;
import com.grpc.user.UserServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

@Configuration
public class GrpcClientsConfig {

    @Bean
    UserServiceGrpc.UserServiceBlockingStub userServiceBlockingStub(GrpcChannelFactory channels) {
        return UserServiceGrpc.newBlockingStub(channels.createChannel("user"));
    }

    @Bean
    NotifyServiceGrpc.NotifyServiceBlockingStub notifyServiceBlockingStub(GrpcChannelFactory channels) {
        return NotifyServiceGrpc.newBlockingStub(channels.createChannel("notify"));
    }
}
