package com.redsafetw.auth_service.grpc;

import com.grpc.notify.NotifyServiceGrpc;
import com.grpc.notify.SendMailVerifyCodeRequest;
import io.grpc.StatusRuntimeException;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

@Component
@RequiredArgsConstructor
@Slf4j
public class NotifyGrpcClient {

    private final NotifyServiceGrpc.NotifyServiceBlockingStub notifyStub;

    public void sendMailVerification(String to, String code, int ttlMinutes) {
        SendMailVerifyCodeRequest request = SendMailVerifyCodeRequest.newBuilder()
                .setTo(to)
                .setCode(code)
                .setTtlMinutes(ttlMinutes)
                .build();
        try {
            notifyStub.sendMailVerifyCode(request);
        } catch (StatusRuntimeException ex) {
            log.error("sendMailVerification gRPC error to={}", to, ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "NOTIFY_GRPC_FAILED", ex);
        }
    }
}
