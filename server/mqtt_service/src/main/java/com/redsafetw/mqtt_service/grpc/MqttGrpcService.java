package com.redsafetw.mqtt_service.grpc;

import com.grpc.mqtt.MqttServiceGrpc;
import com.grpc.mqtt.PublishEdgeCommandRequest;
import com.grpc.mqtt.PublishEdgeCommandResponse;
import com.redsafetw.mqtt_service.service.EdgeMqttPublisher;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.grpc.server.service.GrpcService;
import org.springframework.messaging.MessagingException;

/**
 * gRPC 服務，提供 MQTT 發布能力
 */
@GrpcService
@RequiredArgsConstructor
public class MqttGrpcService extends MqttServiceGrpc.MqttServiceImplBase {
    private static final Logger log = LoggerFactory.getLogger(MqttGrpcService.class);

    private final EdgeMqttPublisher edgeMqttPublisher;

    @Override
    public void publishEdgeCommand(
            PublishEdgeCommandRequest request,
            StreamObserver<PublishEdgeCommandResponse> responseObserver) {
        String edgeId = request.getEdgeId();
        String code = request.getCode();

        try {
            edgeMqttPublisher.publishEdgeCommand(edgeId, code);
            PublishEdgeCommandResponse response = PublishEdgeCommandResponse.newBuilder()
                    .setErrorCode("0")
                    .build();
            responseObserver.onNext(response);
            responseObserver.onCompleted();
        } catch (MessagingException ex) {
            log.error("Failed to publish edge command via MQTT. edgeId={} code={}", edgeId, code, ex);
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

}
