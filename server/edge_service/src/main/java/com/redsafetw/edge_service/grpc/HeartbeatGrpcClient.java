package com.redsafetw.edge_service.grpc;

import com.grpc.heartbeat.HeartbeatServiceGrpc;
import com.grpc.heartbeat.RegisterEdgeHeartbeatRequest;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class HeartbeatGrpcClient {

    private final HeartbeatServiceGrpc.HeartbeatServiceBlockingStub stub;

    public void registerEdgeHeartbeat(String edgeId) {
        RegisterEdgeHeartbeatRequest request = RegisterEdgeHeartbeatRequest.newBuilder()
                .setEdgeId(edgeId)
                .build();
        stub.registerEdgeHeartbeat(request);
    }
}
