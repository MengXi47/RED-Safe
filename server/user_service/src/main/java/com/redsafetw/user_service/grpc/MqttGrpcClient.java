package com.redsafetw.user_service.grpc;

import com.grpc.mqtt.MqttServiceGrpc;
import com.grpc.mqtt.PublishEdgeCommandRequest;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class MqttGrpcClient {

    private final MqttServiceGrpc.MqttServiceBlockingStub stub;

    public String publishEdgeCommand(String edgeId, String code) {
        PublishEdgeCommandRequest request = PublishEdgeCommandRequest.newBuilder()
                .setEdgeId(edgeId)
                .setCode(code)
                .build();
        return stub.publishEdgeCommand(request).getErrorCode();
    }
}
