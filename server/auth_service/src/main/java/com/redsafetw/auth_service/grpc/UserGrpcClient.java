package com.redsafetw.auth_service.grpc;

import com.grpc.user.CreateUserProfileRequest;
import com.grpc.user.CreateUserProfileResponse;
import com.grpc.user.GetUserProfileRequest;
import com.grpc.user.GetUserProfileResponse;
import com.grpc.user.UserServiceGrpc;
import io.grpc.StatusRuntimeException;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.util.Optional;
import java.util.UUID;

@Component
@RequiredArgsConstructor
@Slf4j
public class UserGrpcClient {

    private final UserServiceGrpc.UserServiceBlockingStub userStub;

    public void createUserProfile(UUID userId, String email, String userName) {
        CreateUserProfileRequest request = CreateUserProfileRequest.newBuilder()
                .setUserId(userId.toString())
                .setEmail(email)
                .setUserName(userName == null ? "" : userName)
                .build();
        try {
            CreateUserProfileResponse response = userStub.createUserProfile(request);
            if (!"0".equals(response.getErrorCode())) {
                log.warn("createUserProfile failed user_id={} email={} error={}", userId, email, response.getErrorCode());
                throw new ResponseStatusException(HttpStatus.CONFLICT, response.getErrorCode());
            }
        } catch (StatusRuntimeException ex) {
            log.error("createUserProfile gRPC error user_id={} email={}", userId, email, ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "USER_PROFILE_GRPC_FAILED", ex);
        }
    }

    public Optional<GetUserProfileResponse> getUserProfileByUserId(UUID userId) {
        GetUserProfileRequest request = GetUserProfileRequest.newBuilder()
                .setUserId(userId.toString())
                .build();
        return getUserProfile(request);
    }

    public Optional<GetUserProfileResponse> getUserProfileByEmail(String email) {
        GetUserProfileRequest request = GetUserProfileRequest.newBuilder()
                .setEmail(email)
                .build();
        return getUserProfile(request);
    }

    private Optional<GetUserProfileResponse> getUserProfile(GetUserProfileRequest request) {
        try {
            GetUserProfileResponse response = userStub.getUserProfile(request);
            if (response.getUserId().isBlank()) {
                return Optional.empty();
            }
            return Optional.of(response);
        } catch (StatusRuntimeException ex) {
            if (ex.getStatus().getCode().equals(io.grpc.Status.Code.NOT_FOUND)) {
                return Optional.empty();
            }
            log.error("getUserProfile gRPC error request={}", request, ex);
            throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "USER_PROFILE_GRPC_FAILED", ex);
        }
    }
}
