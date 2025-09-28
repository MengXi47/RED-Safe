package com.redsafetw.user_service.grpc;

import com.grpc.mqtt.MqttServiceGrpc;
import com.grpc.mqtt.PublishEdgeCommandRequest;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class MqttGrpcClient {

    private final MqttServiceGrpc.MqttServiceBlockingStub stub;

    public String publishEdgeCommand(String edgeId, String traceId, String code, String payloadJson) {
        PublishEdgeCommandRequest.Builder builder = PublishEdgeCommandRequest.newBuilder()
                .setEdgeId(edgeId)
                .setCode(code)
                .setTraceId(traceId);

        if (payloadJson != null && !payloadJson.isBlank()) {
            builder.setPayload(payloadJson);
        }

        return stub.publishEdgeCommand(builder.build()).getErrorCode();
    }
}
