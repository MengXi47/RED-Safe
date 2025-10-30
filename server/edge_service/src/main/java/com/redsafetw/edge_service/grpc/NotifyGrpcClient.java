package com.redsafetw.edge_service.grpc;

import com.grpc.notify.NotifyServiceGrpc;
import com.grpc.notify.SendEmailVerifyCodeRequest;
import com.grpc.notify.SendFallAlertRequest;
import io.grpc.StatusRuntimeException;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;

@Component
@RequiredArgsConstructor
@Slf4j
public class NotifyGrpcClient {

    private final NotifyServiceGrpc.NotifyServiceBlockingStub notifyStub;

    public void sendEmailVerification(String to, String code, int ttlMinutes) {
        SendEmailVerifyCodeRequest request = SendEmailVerifyCodeRequest.newBuilder()
                .setTo(to)
                .setCode(code)
                .setTtlMinutes(ttlMinutes)
                .build();
        try {
            notifyStub.sendEmailVerifyCode(request);
        } catch (StatusRuntimeException ex) {
            log.error("sendMailVerification gRPC error to={}", to, ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "NOTIFY_GRPC_FAILED", ex);
        }
    }

    public void sendFallAlertEmail(String to, String edge_id, String ip, String ipc_name) {
        SendFallAlertRequest request = SendFallAlertRequest.newBuilder()
                .setTo(to)
                .setIpAddress(ip)
                .setEdgeId(edge_id)
                .setIpcName(ipc_name)
                .setEventTime(String.valueOf(OffsetDateTime.now()))
                .build();

        try {
            notifyStub.sendFallAlert(request);
        } catch (StatusRuntimeException ex) {
            log.error("sendFallAlertEmail gRPC error to={}", to, ex);
        }
    }
}
