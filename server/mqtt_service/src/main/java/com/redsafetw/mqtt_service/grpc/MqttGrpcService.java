package com.redsafetw.mqtt_service.grpc;

import com.google.protobuf.Empty;
import com.grpc.mqtt.MqttServiceGrpc;
import com.grpc.mqtt.PublishEdgeCommandRequest;
import com.grpc.mqtt.PublishEdgeCommandResponse;
import com.grpc.mqtt.RegisterEdgeStatusRequest;
import com.redsafetw.mqtt_service.service.EdgeHeartbeatManager;
import com.redsafetw.mqtt_service.service.EdgeMqttPublisher;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.grpc.server.service.GrpcService;
import org.springframework.messaging.MessagingException;

/**
 * gRPC 服務，提供 MQTT 發布與訂閱能力
 */
@GrpcService
@RequiredArgsConstructor
public class MqttGrpcService extends MqttServiceGrpc.MqttServiceImplBase {
    private static final Logger log = LoggerFactory.getLogger(MqttGrpcService.class);
    private static final java.util.regex.Pattern EDGE_ID_PATTERN =
            java.util.regex.Pattern.compile("^RED-[0-9A-F]{8}$");

    private final EdgeMqttPublisher edgeMqttPublisher;
    private final EdgeHeartbeatManager edgeHeartbeatManager;

    @Override
    public void publishEdgeCommand(
            PublishEdgeCommandRequest request,
            StreamObserver<PublishEdgeCommandResponse> responseObserver) {
        String edgeId = request.getEdgeId();
        String code = request.getCode();
        String traceId = request.getTraceId();
        String payload = request.getPayload();

        if (!EDGE_ID_PATTERN.matcher(edgeId).matches()) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("edge_id must match RED-[0-9A-F]{8}")
                    .asRuntimeException());
            return;
        }

        try {
            edgeMqttPublisher.publishEdgeCommand(edgeId, traceId, code, payload);
            PublishEdgeCommandResponse response = PublishEdgeCommandResponse.newBuilder()
                    .setErrorCode("0")
                    .build();
            responseObserver.onNext(response);
            responseObserver.onCompleted();
        } catch (MessagingException ex) {
            log.error("Failed to publish edge command via MQTT. edgeId={} traceId={} code={}", edgeId, traceId, code, ex);
            responseObserver.onNext(PublishEdgeCommandResponse.newBuilder()
                    .setErrorCode("MQTT_PUBLISH_FAILED")
                    .build());
            responseObserver.onCompleted();
        } catch (Exception ex) {
            log.error("Unexpected error while publishing MQTT command", ex);
            responseObserver.onError(
                    Status.INTERNAL.withDescription(ex.getMessage()).withCause(ex).asRuntimeException());
        }
    }

    @Override
    public void registerEdgeStatusStream(
            RegisterEdgeStatusRequest request,
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
