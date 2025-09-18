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
    private final EdgeGrpcClient edgeGrpcClient;
    private static final Logger logger = LoggerFactory.getLogger(UserService.class);
    private final AuthService authService;

    public EdgeIdListResponse getEdgeIdList(String accessToken) {

        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("getEdgeIdList: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        List<UserEdgeBindDomain> EdgeIds = userEdgeBindRepository.findByUserId(userId);
        if (EdgeIds.isEmpty()) {
            logger.info("getEdgeIdList: {\"user_id\":\"{}\"} edge_id 數量為 0", userId);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "136");
        }

        List<String> edgeIds = EdgeIds.stream()
                .map(UserEdgeBindDomain::getEdgeId)
                .toList();

        return EdgeIdListResponse.builder()
                .edgeIds(edgeIds)
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

        String errorCode = edgeGrpcClient.UpdataEdgeName(
                updateEdgeNameRequest.getEdgeId(),
                updateEdgeNameRequest.getEdgeName());

        return ErrorCodeResponse.builder()
                .errorCode(errorCode)
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
}