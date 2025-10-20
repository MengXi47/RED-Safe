package com.redsafetw.user_service.grpc;

import com.grpc.auth.AuthServiceGrpc;
import com.grpc.auth.ChangePasswordRequest;
import com.grpc.auth.ChangePasswordResponse;
import com.grpc.auth.UserSecurityProfileRequest;
import com.grpc.auth.UserSecurityProfileResponse;
import com.grpc.auth.accesstokenchkRequset;
import com.grpc.auth.accesstokenchkResponse;
import io.grpc.StatusRuntimeException;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;

@Component
@RequiredArgsConstructor
@Slf4j
public class AuthGrpcClient {

    private final AuthServiceGrpc.AuthServiceBlockingStub authStub;

    public UUID resolveUserId(String accessToken) {
        accesstokenchkRequset request = accesstokenchkRequset.newBuilder()
                .setAccesstoken(accessToken)
                .build();
        try {
            accesstokenchkResponse response = authStub.checkAccessToken(request);
            if (!response.getChk()) {
                return new UUID(0L, 0L);
            }
            return UUID.fromString(response.getUserId());
        } catch (StatusRuntimeException ex) {
            log.error("resolveUserId gRPC error", ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "AUTH_GRPC_FAILED", ex);
        }
    }

    public ChangePasswordResponse changePassword(UUID userId, String currentPassword, String newPassword) {
        ChangePasswordRequest request = ChangePasswordRequest.newBuilder()
                .setUserId(userId.toString())
                .setCurrentPassword(currentPassword)
                .setNewPassword(newPassword)
                .build();
        try {
            return authStub.changePassword(request);
        } catch (StatusRuntimeException ex) {
            log.error("changePassword gRPC error user_id={}", userId, ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "AUTH_GRPC_FAILED", ex);
        }
    }

    public UserSecurityProfileResponse getSecurityProfile(UUID userId) {
        UserSecurityProfileRequest request = UserSecurityProfileRequest.newBuilder()
                .setUserId(userId.toString())
                .build();
        try {
            return authStub.getUserSecurityProfile(request);
        } catch (StatusRuntimeException ex) {
            log.error("getSecurityProfile gRPC error user_id={}", userId, ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "AUTH_GRPC_FAILED", ex);
        }
    }
}
