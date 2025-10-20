package com.redsafetw.auth_service.grpc;

import com.grpc.auth.AuthServiceGrpc;
import com.grpc.auth.ChangePasswordRequest;
import com.grpc.auth.ChangePasswordResponse;
import com.grpc.auth.UserSecurityProfileRequest;
import com.grpc.auth.UserSecurityProfileResponse;
import com.grpc.auth.accesstokenchkRequset;
import com.grpc.auth.accesstokenchkResponse;
import com.redsafetw.auth_service.service.AuthService;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.grpc.server.service.GrpcService;

import java.util.UUID;

@GrpcService
@Slf4j
@RequiredArgsConstructor
public class AuthGrpcServer extends AuthServiceGrpc.AuthServiceImplBase {

    private final AuthService authService;

    @Override
    public void checkAccessToken(accesstokenchkRequset request,
                                 StreamObserver<accesstokenchkResponse> responseObserver) {
        String token = request.getAccesstoken();
        UUID userId = authService.verifyAccessToken(token);
        boolean valid = !userId.equals(new UUID(0L, 0L));

        if (valid) {
            log.debug("checkAccessToken success user_id={}", userId);
        } else {
            log.debug("checkAccessToken invalid token");
        }

        accesstokenchkResponse response = accesstokenchkResponse.newBuilder()
                .setChk(valid)
                .setUserId(valid ? userId.toString() : ".")
                .build();
        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    @Override
    public void changePassword(ChangePasswordRequest request,
                               StreamObserver<ChangePasswordResponse> responseObserver) {
        UUID userId;
        try {
            userId = UUID.fromString(request.getUserId());
        } catch (IllegalArgumentException ex) {
            ChangePasswordResponse response = ChangePasswordResponse.newBuilder()
                    .setSuccess(false)
                    .setErrorCode("142")
                    .build();
            responseObserver.onNext(response);
            responseObserver.onCompleted();
            return;
        }

        var result = authService.changePassword(userId, request.getCurrentPassword(), request.getNewPassword());

        ChangePasswordResponse response = ChangePasswordResponse.newBuilder()
                .setSuccess(result.success())
                .setErrorCode(result.errorCode())
                .build();
        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    @Override
    public void getUserSecurityProfile(UserSecurityProfileRequest request,
                                       StreamObserver<UserSecurityProfileResponse> responseObserver) {
        UUID userId;
        try {
            userId = UUID.fromString(request.getUserId());
        } catch (IllegalArgumentException ex) {
            responseObserver.onNext(UserSecurityProfileResponse.newBuilder()
                    .setOtpEnabled(false)
                    .setEmailVerified(false)
                    .setLastLoginAt("")
                    .build());
            responseObserver.onCompleted();
            return;
        }

        var profileOpt = authService.getUserSecurityProfile(userId);
        UserSecurityProfileResponse response = profileOpt.map(profile -> UserSecurityProfileResponse.newBuilder()
                        .setOtpEnabled(profile.otpEnabled())
                        .setEmailVerified(profile.emailVerified())
                        .setLastLoginAt(profile.lastLoginIso())
                        .build())
                .orElse(UserSecurityProfileResponse.newBuilder()
                        .setOtpEnabled(false)
                        .setEmailVerified(false)
                        .setLastLoginAt("")
                        .build());
        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }
}
