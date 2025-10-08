package com.redsafetw.heartbeat_service.grpc;

import com.google.protobuf.Empty;
import com.grpc.heartbeat.HeartbeatServiceGrpc;
import com.grpc.heartbeat.RegisterEdgeHeartbeatRequest;
import com.redsafetw.heartbeat_service.service.EdgeHeartbeatManager;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.grpc.server.service.GrpcService;

import java.util.regex.Pattern;

@GrpcService
@RequiredArgsConstructor
public class HeartbeatGrpcService extends HeartbeatServiceGrpc.HeartbeatServiceImplBase {
    private static final Logger log = LoggerFactory.getLogger(HeartbeatGrpcService.class);
    private static final Pattern EDGE_ID_PATTERN = Pattern.compile("^RED-[0-9A-F]{8}$");

    private final EdgeHeartbeatManager edgeHeartbeatManager;

    @Override
    public void registerEdgeHeartbeat(RegisterEdgeHeartbeatRequest request,
                                      StreamObserver<Empty> responseObserver) {
        String edgeId = request.getEdgeId();
        if (!EDGE_ID_PATTERN.matcher(edgeId).matches()) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("edge_id must match RED-[0-9A-F]{8}")
                    .asRuntimeException());
            return;
        }

        try {
            edgeHeartbeatManager.registerEdge(edgeId);
            responseObserver.onNext(Empty.getDefaultInstance());
            responseObserver.onCompleted();
        } catch (Exception ex) {
            log.error("Failed to register heartbeat stream for edge {}", edgeId, ex);
            responseObserver.onError(
                    Status.INTERNAL.withDescription(ex.getMessage()).withCause(ex).asRuntimeException());
        }
    }
}
