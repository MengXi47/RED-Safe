package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.grpc.EdgeGrpcClient;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.repository.UserRepository;
import com.redsafetw.user_service.util.*;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.util.List;
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
    private static final Logger logger = LoggerFactory.getLogger(UserService.class);
    private final AuthService authService;
    private final EdgeGrpcClient edgeGrpcClient;

    public EdgeIdListResponse getEdgeIdList(String accessToken) {

        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("getEdgeIdList: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        List<UserEdgeBindDomain> edgeBindings = userEdgeBindRepository.findByUserId(userId);
        if (edgeBindings.isEmpty()) {
            logger.info("getEdgeIdList: {\"user_id\":\"{}\"} edge_id 數量為 0", userId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "136");
        }

        List<EdgeIdListResponse.EdgeItem> edges = edgeBindings.stream()
                .map(bind -> EdgeIdListResponse.EdgeItem.builder()
                        .edgeId(bind.getEdgeId())
                        .displayName(bind.getDisplayName())
                        .build())
                .toList();

        return EdgeIdListResponse.builder()
                .edges(edges)
                .build();
    }

    public ErrorCodeResponse updataEdgeName(
            UpdateEdgeNameRequest updateEdgeNameRequest,
            String accessToken) {

        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("updataEdgeName: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

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
        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("updateUserName: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }


        var user = userRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        user.setUser_name(updateUserNameRequest.getUserName());
        userRepository.save(user);

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public ErrorCodeResponse updateUserPassword(
            UpdateUserPasswordRequest updateUserPasswordRequest,
            String accessToken) {
        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("updateUserPassword: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        var user = userRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        if (!authService.verifyPassword(userId, updateUserPasswordRequest.getPassword())) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "143");
        }

        String newPasswordHash = Argon2id.hash(updateUserPasswordRequest.getNewPassword());

        user.setUser_password_hash(newPasswordHash);
        userRepository.save(user);

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public ErrorCodeResponse updateEdgePassword(
            UpdateEdgePasswordRequest updateEdgePasswordRequest,
            String accessToken) {
        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("updateEdgePassword: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

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
