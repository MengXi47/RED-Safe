package com.redsafetw.edge_service.grpc;

import com.grpc.heartbeat.HeartbeatServiceGrpc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.grpc.client.GrpcChannelFactory;

@Configuration
public class GrpcClientsConfig {

    @Bean
    HeartbeatServiceGrpc.HeartbeatServiceBlockingStub heartbeatBlockingStub(GrpcChannelFactory channels) {
        return HeartbeatServiceGrpc.newBlockingStub(channels.createChannel("heartbeat"));
    }
}
