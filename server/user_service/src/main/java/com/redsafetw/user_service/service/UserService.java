package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.domain.UserDomain;
import com.redsafetw.user_service.grpc.EdgeGrpcClient;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.util.*;
import com.redsafetw.user_service.repository.UserRepository;
import com.redsafetw.user_service.repository.AuthRepository;
import com.redsafetw.user_service.domain.AuthDomain;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;
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
    private final UserRepository users;
    private final AuthRepository auths;
    private final UserEdgeBindRepository userEdgeBindRepository;
    private final EdgeGrpcClient edgeGrpcClient;
    private static final Logger logger = LoggerFactory.getLogger(UserService.class);

    public SignupResponse signup(SignupRequest req) {

        // 驗證Email使否重複
        if (users.existsByEmail(req.getEmail())) {
            logger.info("Signup: {\"user_name\":\"{}\", \"email\":\"{}\", \"password\":\"{}\"} Email已存在",
                    req.getUserName(),
                    req.getEmail(),
                    req.getPassword());
            throw new ResponseStatusException(HttpStatus.CONFLICT, "133");
        }

        // 寫入資料庫
        UserDomain user = new UserDomain();
        user.setEmail(req.getEmail());
        user.setUser_name(req.getUserName());
        user.setUser_password_hash(Argon2id.hash(req.getPassword()));
        user.setStatus(true);
        user = users.save(user);

        logger.info("Signup: {\"user_name\":\"{}\", \"email\":\"{}\", \"password\":\"{}\"} Signup successful user_id: {}",
                user.getUser_name(),
                user.getEmail(),
                req.getPassword(),
                user.getUser_id());
        return SignupResponse.builder()
                .userId(user.getUser_id())
                .userName(user.getUser_name())
                .build();
    }

    public SigninResponse signin(SigninRequest req) {

        // 以 Email 取得使用者
        var userOpt = users.findByEmail(req.getEmail().trim());
        if (userOpt.isEmpty()) {
            logger.info("Signin: {\"email\":\"{}\", \"password\":\"{}\"} 帳號密碼錯誤", req.getEmail(), req.getPassword());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        var user = userOpt.get();

        // 驗證密碼
        boolean ok = Argon2id.verify(user.getUser_password_hash(), req.getPassword());
        if (!ok) {
            logger.info("Signin: {\"email\":\"{}\", \"password\":\"{}\"} 帳號密碼錯誤", req.getEmail(), req.getPassword());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        String accessToken = JWT.createToken(user.getUser_id());
        String refreshToken = RefreshToken.generateRefreshToken();

        user.setLast_login_at(OffsetDateTime.now());
        users.save(user);

        AuthDomain auth = new AuthDomain();
        auth.setUser(user);
        auth.setRefresh_token(refreshToken);
        auth.setCreated_at(OffsetDateTime.now());
        auth.setExpires_at(OffsetDateTime.now().plusDays(30));
        auth.setRevoked(false);
        auths.save(auth);

        logger.info("Signin: {\"email\":\"{}\", \"password\":\"{}\"} Signin successful user_id: {}",
                req.getEmail(),
                req.getPassword(),
                user.getUser_id());
        return SigninResponse.builder().userName(user.getUser_name()).accessToken(accessToken).refreshToken(refreshToken).build();
    }

    public EdgeIdListResponse getEdgeIdList(String accessToken) {

        UUID userId = JWT.verifyAndGetUserId(accessToken);
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

    public UpdataEdgeNameResponse updataEdgeName(
            UpdataEdgeNameRequest updataEdgeNameRequest,
            String accessToken) {

        UUID userId = JWT.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("updataEdgeName: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, updataEdgeNameRequest.getEdgeId())) {
            logger.info("updataEdgeName: {\"user_id\":\"{}\", \"edge_id\":\"{}\"} 未綁定",
                    userId, updataEdgeNameRequest.getEdgeId());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        String errorCode = edgeGrpcClient.UpdataEdgeName(
                updataEdgeNameRequest.getEdgeId(),
                updataEdgeNameRequest.getEdgeName());

        return UpdataEdgeNameResponse.builder()
                .errorCode(errorCode)
                .build();
    }
}