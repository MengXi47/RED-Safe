package com.redsafetw.user_service.service;

import com.grpc.edge.EdgeServiceGrpc;
import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.grpc.EdgeGrpcClient;
import com.redsafetw.user_service.repository.AuthRepository;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.util.*;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.util.Scanner;
import java.util.UUID;

/**
 * 綁定服務
 *
 * @create 2025-09-14
 */
@Service
@Transactional
@RequiredArgsConstructor
public class BindService {

    private final EdgeGrpcClient edgeGrpcClient;
    private final UserEdgeBindRepository userEdgeBindRepository;
    private static final Logger logger = LoggerFactory.getLogger(BindService.class);

    public BindResponse bind(String access_token, String edge_id) {

        UUID userId = JWT.verifyAndGetUserId(access_token);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} access_token 失效", userId, edge_id);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        if (!edgeGrpcClient.CheckEdgeIdExists(edge_id)) {
            logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} edge_id 不存在", userId, edge_id);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "123");
        }

        if (userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edge_id)) {
            logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 已綁定", userId, edge_id);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "134");
        }

        UserEdgeBindDomain userEdgeBindDomain = new UserEdgeBindDomain();
        userEdgeBindDomain.setUserId(userId);
        userEdgeBindDomain.setEdgeId(edge_id);
        userEdgeBindRepository.save(userEdgeBindDomain);

        logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} Bind successful", userId, edge_id);
        return BindResponse.builder()
                .errorCode("0")
                .build();
    }

    public UnbindResponse unbind(String access_token, String edge_id) {

        UUID userId = JWT.verifyAndGetUserId(access_token);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("UserEdgeUnbind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} access_token 失效", userId, edge_id);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        if (!edgeGrpcClient.CheckEdgeIdExists(edge_id)) {
            logger.info("UserEdgeUnbind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} edge_id 不存在", userId, edge_id);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "123");
        }

        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edge_id)) {
            logger.info("UserEdgeUnbind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定", userId, edge_id);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        userEdgeBindRepository.deleteByUserIdAndEdgeId(userId, edge_id);

        logger.info("UserEdgeUnbind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} Unbind successful", userId, edge_id);
        return UnbindResponse.builder()
                .errorCode("0")
                .build();
    }
}