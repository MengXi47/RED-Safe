package com.redsafetw.edge_service.service;

import com.grpc.user.ListEdgeUsersResponse;
import com.redsafetw.edge_service.dto.EdgeUserBindListResponse;
import com.redsafetw.edge_service.dto.ErrorCodeResponse;
import com.redsafetw.edge_service.grpc.UserGrpcClient;
import com.redsafetw.edge_service.repository.EdgeRepository;
import io.grpc.Status;
import io.grpc.StatusRuntimeException;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;

/**
 * 查詢綁定使用者服務
 */
@Service
@RequiredArgsConstructor
public class EdgeUserBindService {

    private static final Logger log = LoggerFactory.getLogger(EdgeUserBindService.class);
    private static final DateTimeFormatter ISO_FORMATTER = DateTimeFormatter.ISO_OFFSET_DATE_TIME;

    private final EdgeRepository edgeRepository;
    private final UserGrpcClient userGrpcClient;

    public EdgeUserBindListResponse getUserList(String edgeId) {
        var device = edgeRepository.findByEdgeId(edgeId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "123"));

        ListEdgeUsersResponse grpcResponse;
        try {
            grpcResponse = userGrpcClient.listEdgeUsers(edgeId);
        } catch (StatusRuntimeException ex) {
            log.error("Failed to fetch edge users via gRPC, edge_id={}, status={}", edgeId, ex.getStatus(), ex);
            throw translateGrpcError(ex);
        }

        String lastOnline = formatDate(device.getLastOnlineAt());
        List<EdgeUserBindListResponse.UserItem> users = grpcResponse.getUsersList().stream()
                .map(user -> EdgeUserBindListResponse.UserItem.builder()
                        .userId(user.getUserId())
                        .email(user.getEmail())
                        .userName(user.getUserName())
                        .bindAt(user.getBindAt())
                        .lastOnline(user.getLastOnline().isBlank() ? lastOnline : user.getLastOnline())
                        .build())
                .toList();

        return EdgeUserBindListResponse.builder()
                .users(users)
                .build();
    }

    public ErrorCodeResponse unbindUser(String edgeId, String userId, String email) {
        // 確認 edge 存在
        edgeRepository.findByEdgeId(edgeId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "123"));

        try {
            var response = userGrpcClient.unbindEdgeUser(edgeId, userId, email);
            return ErrorCodeResponse.builder()
                    .errorCode(response.getErrorCode())
                    .build();
        } catch (StatusRuntimeException ex) {
            log.error("Failed to unbind user via gRPC, edge_id={}, user_id={}, email={}, status={}",
                    edgeId, userId, email, ex.getStatus(), ex);
            throw translateGrpcError(ex);
        }
    }

    private ResponseStatusException translateGrpcError(StatusRuntimeException ex) {
        Status.Code code = ex.getStatus().getCode();
        String reason = ex.getStatus().getDescription() == null
                ? code.name()
                : ex.getStatus().getDescription();

        HttpStatus status = switch (code) {
            case INVALID_ARGUMENT -> HttpStatus.BAD_REQUEST;
            case NOT_FOUND -> HttpStatus.NOT_FOUND;
            case PERMISSION_DENIED, UNAUTHENTICATED -> HttpStatus.UNAUTHORIZED;
            default -> HttpStatus.SERVICE_UNAVAILABLE;
        };
        return new ResponseStatusException(status, reason, ex);
    }

    private String formatDate(OffsetDateTime dateTime) {
        return dateTime == null ? "" : ISO_FORMATTER.format(dateTime);
    }
}
