package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.UserDomain;
import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.repository.UserRepository;
import com.redsafetw.user_service.util.*;
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
import java.util.Optional;
import java.util.UUID;

/**
 * 認證服務
 *
 * @create 2025-09-13
 */
@Service
@Transactional
@RequiredArgsConstructor
public class AuthService {
    private final UserRepository users;
    private final AuthRepository auths;
    private final AuthRepository authRepository;
    private static final Logger logger = LoggerFactory.getLogger(AuthService.class);

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
                user.getUserId());
        return SignupResponse.builder()
                .userId(user.getUserId())
                .userName(user.getUser_name())
                .build();
    }

    public SigninResponse signin(SigninRequest req) {

        // 驗證帳號與密碼
        if (!verifyPassword(req.getEmail(), req.getPassword())) {
            logger.info("Signin: {\"email\":\"{}\", \"password\":\"{}\"} 帳號密碼錯誤", req.getEmail(), req.getPassword());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        var user = users.findByEmail(req.getEmail().trim())
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128"));

        String accessToken = JwtService.createToken(user.getUserId());
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
                user.getUserId());
        return SigninResponse.builder().userName(user.getUser_name()).accessToken(accessToken).refreshToken(refreshToken).build();
    }

    public RefreshResponse refresh(RefreshRequest req) {

        OffsetDateTime now = OffsetDateTime.now();
        Optional<AuthDomain> result = authRepository.findActiveByRefreshToken(req.getRefreshToken(), now);

        if (result.isEmpty()) {
            logger.info("Refresh: {\"refresh_token\":\"{}\"} refresh_token 失效",
                    req.getRefreshToken());
            throw new ResponseStatusException(HttpStatus.CONFLICT, "132");
        }

        AuthDomain auth = result.get();
        UUID user_id = auth.getUser().getUserId();
        String access_token = JwtService.createToken(user_id);

        var user = auth.getUser();
        user.setLast_login_at(OffsetDateTime.now());

        logger.info("Refresh: {\"refresh_token\":\"{}\"} Refresh successful user_id: {}",
                req.getRefreshToken(),
                user_id);
        return RefreshResponse.builder()
                .accessToken(access_token)
                .build();
    }

    /**
     * 驗證密碼（以 Email 查詢）
     * @param email 使用者 Email
     * @param rawPassword 原始密碼
     * @return 若密碼正確則回傳 true，否則 false；查無使用者亦回傳 false
     */
    public boolean verifyPassword(String email, String rawPassword) {
        if (email == null || rawPassword == null) return false;
        var userOpt = users.findByEmail(email.trim());
        if (userOpt.isEmpty()) {
            return false;
        }
        var user = userOpt.get();
        return Argon2id.verify(user.getUser_password_hash(), rawPassword);
    }

    /**
     * 驗證密碼（以 user_id 查詢）
     * @param userId 使用者 ID
     * @param rawPassword 原始密碼
     * @return 若密碼正確則回傳 true，否則 false；查無使用者亦回傳 false
     */
    public boolean verifyPassword(UUID userId, String rawPassword) {
        if (userId == null || rawPassword == null) return false;
        var userOpt = users.findByUserId(userId);
        if (userOpt.isEmpty()) {
            return false;
        }
        var user = userOpt.get();
        return Argon2id.verify(user.getUser_password_hash(), rawPassword);
    }
}
