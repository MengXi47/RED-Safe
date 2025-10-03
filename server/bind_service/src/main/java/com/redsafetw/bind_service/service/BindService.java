package com.redsafetw.bind_service.service;

import com.redsafetw.bind_service.domain.UserEdgeBindDomain;
import com.redsafetw.bind_service.dto.BindRequest;
import com.redsafetw.bind_service.dto.ErrorCodeResponse;
import com.redsafetw.bind_service.dto.UnBindRequest;
import com.redsafetw.bind_service.grpc.AuthGrpcClient;
import com.redsafetw.bind_service.grpc.EdgeGrpcClient;
import com.redsafetw.bind_service.repository.UserEdgeBindRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;

@Service
@Transactional
@RequiredArgsConstructor
public class BindService {
    private final EdgeGrpcClient edgeGrpcClient;
    private final AuthGrpcClient authGrpcClient;
    private final UserEdgeBindRepository userEdgeBindRepository;
    private static final Logger logger = LoggerFactory.getLogger(BindService.class);
    public ErrorCodeResponse BindUserEdge(String token, BindRequest bindRequest) {
        String userId = authGrpcClient.chkAccessToken(token);
        if (userId == null) {
            logger.info("UserEdgeBind: {\"access_token\":\"{}\", \"edge_id\":\"{}\"} access_token 失效",
                    token, bindRequest.getEdgeId());
            return ErrorCodeResponse.builder().errorCode("126").build();
        }

        if (!edgeGrpcClient.CheckEdgeIdExists(bindRequest.getEdgeId())) {
            logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} edge_id 不存在",
                    userId, bindRequest.getEdgeId());
            return ErrorCodeResponse.builder().errorCode("123").build();
        }

        if (!edgeGrpcClient.verifyEdgeCredentials(bindRequest.getEdgeId(), bindRequest.getEdgePassword())) {
            logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} edge 密碼驗證失敗",
                    userId, bindRequest.getEdgeId());
            return ErrorCodeResponse.builder().errorCode("147").build();
        }

        if (userEdgeBindRepository.existsByUserIdAndEdgeId(UUID.fromString(userId), bindRequest.getEdgeId())) {
            logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 已綁定",
                    userId, bindRequest.getEdgeId());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "134");
        }

        UserEdgeBindDomain userEdgeBindDomain = new UserEdgeBindDomain();
        userEdgeBindDomain.setUserId(UUID.fromString(userId));
        userEdgeBindDomain.setEdgeId(bindRequest.getEdgeId());
        userEdgeBindDomain.setDisplayName(bindRequest.getEdgeName());
        userEdgeBindDomain.setIsOnline(false);
        userEdgeBindRepository.save(userEdgeBindDomain);

        logger.info("UserEdgeBind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} Bind successful",
                userId, bindRequest.getEdgeId());
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public ErrorCodeResponse UnBindUserEdge(String token, UnBindRequest unBindRequest) {
        String userId = authGrpcClient.chkAccessToken(token);
        if (userId == null) {
            logger.info("UnBindUserEdge: {\"access_token\":\"{}\", \"edge_id\":\"{}\"} access_token 失效",
                    token, unBindRequest.getEdgeId());
            return ErrorCodeResponse.builder().errorCode("126").build();
        }

        if (!edgeGrpcClient.CheckEdgeIdExists(unBindRequest.getEdgeId())) {
            logger.info("UnBindUserEdge: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} edge_id 不存在", userId, unBindRequest.getEdgeId());
            return ErrorCodeResponse.builder().errorCode("123").build();
        }

        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(UUID.fromString(userId), unBindRequest.getEdgeId())) {
            logger.info("UnBindUserEdge: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定", userId, unBindRequest.getEdgeId());
            return ErrorCodeResponse.builder().errorCode("135").build();
        }

        userEdgeBindRepository.deleteByUserIdAndEdgeId(UUID.fromString(userId), unBindRequest.getEdgeId());

        logger.info("UserEdgeUnbind: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} Unbind successful", userId, unBindRequest.getEdgeId());
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }
}
