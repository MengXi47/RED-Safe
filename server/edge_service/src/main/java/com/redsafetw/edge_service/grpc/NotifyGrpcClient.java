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
import java.time.format.DateTimeFormatter;

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
        OffsetDateTime now = OffsetDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy/MM/dd HH:mm:ss");
        String formatted = now.format(formatter);

        SendFallAlertRequest request = SendFallAlertRequest.newBuilder()
                .setTo(to)
                .setIpAddress(ip)
                .setEdgeId(edge_id)
                .setIpcName(ipc_name)
                .setLocation("臺中市太平區中山路二段57號")
                .setEventTime(formatted)
                .build();

        try {
            notifyStub.sendFallAlert(request);
        } catch (StatusRuntimeException ex) {
            log.error("sendFallAlertEmail gRPC error to={}", to, ex);
        }
    }
}
