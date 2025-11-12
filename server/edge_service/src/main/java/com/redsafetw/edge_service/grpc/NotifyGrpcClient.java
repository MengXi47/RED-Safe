package com.redsafetw.edge_service.grpc;

import com.google.protobuf.ByteString;
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
import java.util.Optional;

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

    public void sendFallAlertEmail(
            String to,
            String edgeId,
            String ip,
            String ipcName,
            OffsetDateTime eventTime,
            String location,
            byte[] snapshot,
            String snapshotMimeType
    ) {
        OffsetDateTime occurTime = Optional.ofNullable(eventTime).orElse(OffsetDateTime.now());
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy/MM/dd HH:mm:ss");
        String formatted = occurTime.format(formatter);

        ByteString snapshotBytes = snapshot == null ? ByteString.EMPTY : ByteString.copyFrom(snapshot);
        String mimeType = snapshotMimeType == null || snapshotMimeType.isBlank() ? "image/jpeg" : snapshotMimeType;
        String safeLocation = (location == null || location.isBlank())
                ? "臺中市太平區中山路二段57號"
                : location;

        SendFallAlertRequest request = SendFallAlertRequest.newBuilder()
                .setTo(to)
                .setIpAddress(ip)
                .setEdgeId(edgeId)
                .setIpcName(ipcName == null ? "" : ipcName)
                .setLocation(safeLocation)
                .setEventTime(formatted)
                .setSnapshot(snapshotBytes)
                .setSnapshotMimeType(mimeType)
                .build();

        try {
            notifyStub.sendFallAlert(request);
        } catch (StatusRuntimeException ex) {
            log.error("sendFallAlertEmail gRPC error to={}", to, ex);
        }
    }
}
