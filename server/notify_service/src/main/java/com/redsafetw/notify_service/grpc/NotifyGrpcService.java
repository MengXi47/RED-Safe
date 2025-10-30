package com.redsafetw.notify_service.grpc;

import com.google.protobuf.Empty;
import com.grpc.notify.NotifyServiceGrpc;
import com.grpc.notify.SendFallAlertRequest;
import com.grpc.notify.SendEmailVerifyCodeRequest;
import com.redsafetw.notify_service.service.EmailService;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import jakarta.mail.MessagingException;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.grpc.server.service.GrpcService;
import org.springframework.util.StringUtils;

@GrpcService
@RequiredArgsConstructor
public class NotifyGrpcService extends NotifyServiceGrpc.NotifyServiceImplBase {
    private static final Logger log = LoggerFactory.getLogger(NotifyGrpcService.class);

    private final EmailService emailService;

    @Override
    public void sendEmailVerifyCode(SendEmailVerifyCodeRequest request, StreamObserver<Empty> responseObserver) {
        String to = request.getTo();
        String code = request.getCode();
        int ttlMinutes = request.getTtlMinutes();

        log.info("gRPC Notify.SendMailVerifyCode to={}", to);

        if (!StringUtils.hasText(to) || !StringUtils.hasText(code)) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("to and code must not be blank")
                    .asRuntimeException());
            return;
        }

        if (ttlMinutes <= 0) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("ttl_minutes must be greater than zero")
                    .asRuntimeException());
            return;
        }

        try {
            emailService.sendEmailVerifyCode(to, code, ttlMinutes);
            responseObserver.onNext(Empty.getDefaultInstance());
            responseObserver.onCompleted();
        } catch (MessagingException ex) {
            log.error("Failed to send verification code email to {}", to, ex);
            responseObserver.onError(Status.INTERNAL
                    .withDescription("Failed to send verification code email")
                    .withCause(ex)
                    .asRuntimeException());
        } catch (Exception ex) {
            log.error("Unexpected error while sending verification code email to {}", to, ex);
            responseObserver.onError(Status.UNKNOWN
                    .withDescription("Unexpected error occurred")
                    .withCause(ex)
                    .asRuntimeException());
        }
    }

    @Override
    public void sendFallAlert(SendFallAlertRequest request, StreamObserver<Empty> responseObserver) {
        String to = request.getTo();
        String edgeId = request.getEdgeId();
        String ipAddress = request.getIpAddress();
        String ipcName = request.getIpcName();
        String eventTime = request.getEventTime();
        String location = request.getLocation();

        log.info("gRPC Notify.SendFallAlert to={} patientId={}", to, edgeId);

        if (!StringUtils.hasText(to) || !StringUtils.hasText(edgeId) || !StringUtils.hasText(eventTime)) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("to, patient_id and event_time must not be blank")
                    .asRuntimeException());
            return;
        }

        try {
            emailService.sendFallAlert(to, edgeId, ipAddress, ipcName, eventTime, location);
            responseObserver.onNext(Empty.getDefaultInstance());
            responseObserver.onCompleted();
        } catch (MessagingException ex) {
            log.error("Failed to send fall alert email to {}", to, ex);
            responseObserver.onError(Status.INTERNAL
                    .withDescription("Failed to send fall alert email")
                    .withCause(ex)
                    .asRuntimeException());
        } catch (Exception ex) {
            log.error("Unexpected error while sending fall alert email to {}", to, ex);
            responseObserver.onError(Status.UNKNOWN
                    .withDescription("Unexpected error occurred")
                    .withCause(ex)
                    .asRuntimeException());
        }
    }
}
