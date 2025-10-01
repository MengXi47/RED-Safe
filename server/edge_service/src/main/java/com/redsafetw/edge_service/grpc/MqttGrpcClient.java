package com.redsafetw.edge_service.grpc;

import com.grpc.mqtt.MqttServiceGrpc;
import com.grpc.mqtt.RegisterEdgeStatusRequest;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class MqttGrpcClient {

    private final MqttServiceGrpc.MqttServiceBlockingStub stub;

    public void registerEdgeStatusStream(String edgeId) {
        RegisterEdgeStatusRequest request = RegisterEdgeStatusRequest.newBuilder()
                .setEdgeId(edgeId)
                .build();
        stub.registerEdgeStatusStream(request);
    }
}

