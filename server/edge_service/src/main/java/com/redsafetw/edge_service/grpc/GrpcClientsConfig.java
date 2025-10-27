package com.redsafetw.edge_service.grpc;

import com.grpc.fall.FallInferenceServiceGrpc;
import com.grpc.heartbeat.HeartbeatServiceGrpc;
import com.grpc.user.UserServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

@Configuration
public class GrpcClientsConfig {

    @Bean
    HeartbeatServiceGrpc.HeartbeatServiceBlockingStub heartbeatBlockingStub(GrpcChannelFactory channels) {
        return HeartbeatServiceGrpc.newBlockingStub(channels.createChannel("heartbeat"));
    }

    @Bean
    UserServiceGrpc.UserServiceBlockingStub userBlockingStub(GrpcChannelFactory channels) {
        return UserServiceGrpc.newBlockingStub(channels.createChannel("user"));
    }

    @Bean
    FallInferenceServiceGrpc.FallInferenceServiceBlockingStub fallInferenceBlockingStub(GrpcChannelFactory channels) {
        return FallInferenceServiceGrpc.newBlockingStub(channels.createChannel("fallInference"));
    }
}
