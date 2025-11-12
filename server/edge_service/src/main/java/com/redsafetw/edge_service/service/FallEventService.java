package com.redsafetw.edge_service.service;

import com.grpc.user.EdgeUserBind;
import com.grpc.user.ListEdgeUsersResponse;
import com.redsafetw.edge_service.dto.ErrorCodeResponse;
import com.redsafetw.edge_service.dto.FallEventReportRequest;
import com.redsafetw.edge_service.grpc.NotifyGrpcClient;
import com.redsafetw.edge_service.grpc.UserGrpcClient;
import io.grpc.StatusRuntimeException;
import java.util.Base64;
import java.util.List;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;
import org.springframework.web.server.ResponseStatusException;

@Service
@RequiredArgsConstructor
@Slf4j
public class FallEventService {

    private final UserGrpcClient userGrpcClient;
    private final NotifyGrpcClient notifyGrpcClient;

    public ErrorCodeResponse handleFallEvent(FallEventReportRequest request) {
        ListEdgeUsersResponse grpcResponse;
        try {
            grpcResponse = userGrpcClient.listEdgeUsers(request.getEdgeId());
        } catch (StatusRuntimeException ex) {
            log.error("listEdgeUsers failed for edge {}", request.getEdgeId(), ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "user_lookup_failed", ex);
        }

        List<EdgeUserBind> users = grpcResponse.getUsersList();
        byte[] snapshotBytes = decodeSnapshot(request.getSnapshotBase64());

        users.forEach(user -> notifyGrpcClient.sendFallAlertEmail(
                user.getEmail(),
                request.getEdgeId(),
                request.getIpAddress(),
                request.getIpcName(),
                request.getEventTime(),
                request.getLocation(),
                snapshotBytes,
                request.getSnapshotMimeType()
        ));

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    private byte[] decodeSnapshot(String base64) {
        if (!StringUtils.hasText(base64)) {
            return null;
        }
        try {
            return Base64.getDecoder().decode(base64);
        } catch (IllegalArgumentException ex) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "invalid_snapshot_base64", ex);
        }
    }
}
