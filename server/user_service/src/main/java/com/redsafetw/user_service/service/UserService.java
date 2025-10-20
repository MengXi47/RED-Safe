package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.dto.*;
import com.grpc.auth.ChangePasswordResponse;
import com.grpc.auth.UserSecurityProfileResponse;
import com.redsafetw.user_service.grpc.AuthGrpcClient;
import com.redsafetw.user_service.grpc.EdgeGrpcClient;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.repository.UserRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;


/**
 * 使用者服務
 *
 * @create 2025-09-11 11:49 pm
 */
@Service
@Transactional
@RequiredArgsConstructor
public class UserService {
    private final UserRepository userRepository;
    private final UserEdgeBindRepository userEdgeBindRepository;
    private final AuthGrpcClient authGrpcClient;
    private final TokenVerifier tokenVerifier;
    private final EdgeGrpcClient edgeGrpcClient;
    private static final Logger logger = LoggerFactory.getLogger(UserService.class);

    public UserInfoResponse getUserInfo(String accessToken) {
        UUID userId = tokenVerifier.requireUserId(accessToken);

        var user = userRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        UserSecurityProfileResponse profile = authGrpcClient.getSecurityProfile(userId);

        return UserInfoResponse.builder()
                .userName(user.getUserName())
                .email(user.getEmail())
                .otpEnabled(profile.getOtpEnabled())
                .build();
    }

    public ErrorCodeResponse updataEdgeName(
            UpdateEdgeNameRequest updateEdgeNameRequest,
            String accessToken) {

        UUID userId = tokenVerifier.requireUserId(accessToken);

        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, updateEdgeNameRequest.getEdgeId())) {
            logger.info("updataEdgeName: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定",
                    userId, updateEdgeNameRequest.getEdgeId());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        UserEdgeBindDomain.BindId bindId = new UserEdgeBindDomain.BindId();
        bindId.setUserId(userId);
        bindId.setEdgeId(updateEdgeNameRequest.getEdgeId());

        UserEdgeBindDomain userEdgeBind = userEdgeBindRepository.findById(bindId)
                .orElseThrow(() -> {
                    logger.info("updataEdgeName: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 紀錄不存在",
                            userId, updateEdgeNameRequest.getEdgeId());
                    return new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
                });

        userEdgeBind.setDisplayName(updateEdgeNameRequest.getEdgeName());
        userEdgeBindRepository.save(userEdgeBind);


        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public ErrorCodeResponse updateUserName(
            UpdateUserNameRequest updateUserNameRequest,
            String accessToken) {
        UUID userId = tokenVerifier.requireUserId(accessToken);


        var user = userRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        user.setUserName(updateUserNameRequest.getUserName());
        userRepository.save(user);

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public ErrorCodeResponse updateUserPassword(
            UpdateUserPasswordRequest updateUserPasswordRequest,
            String accessToken) {
        UUID userId = tokenVerifier.requireUserId(accessToken);

        userRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        ChangePasswordResponse response = authGrpcClient.changePassword(
                userId,
                updateUserPasswordRequest.getPassword(),
                updateUserPasswordRequest.getNewPassword());

        if (!response.getSuccess()) {
            HttpStatus status = switch (response.getErrorCode()) {
                case "142", "143" -> HttpStatus.UNAUTHORIZED;
                default -> HttpStatus.BAD_REQUEST;
            };
            throw new ResponseStatusException(status, response.getErrorCode());
        }

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public ErrorCodeResponse updateEdgePassword(
            UpdateEdgePasswordRequest updateEdgePasswordRequest,
            String accessToken) {
        UUID userId = tokenVerifier.requireUserId(accessToken);

        String edgeId = updateEdgePasswordRequest.getEdgeId();
        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edgeId)) {
            logger.info("updateEdgePassword: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定", userId, edgeId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        String errorCode = edgeGrpcClient.updateEdgePassword(
                edgeId,
                updateEdgePasswordRequest.getEdgePassword(),
                updateEdgePasswordRequest.getNewEdgePassword());

        if (!"0".equals(errorCode)) {
            logger.info("updateEdgePassword: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} gRPC error {}", userId, edgeId, errorCode);
            HttpStatus status = switch (errorCode) {
                case "123", "147" -> HttpStatus.UNAUTHORIZED;
                default -> HttpStatus.BAD_REQUEST;
            };
            throw new ResponseStatusException(status, errorCode);
        }

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }
}
