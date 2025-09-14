package com.redsafetw.user_service.service;

import com.redsafetw.user_service.dto.*;
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

    private final AuthRepository authRepository;
    private static final Logger logger = LoggerFactory.getLogger(AuthService.class);

    public RefreshResponse refresh(RefreshRequest req) {

        OffsetDateTime now = OffsetDateTime.now();
        Optional<AuthDomain> result = authRepository.findActiveByRefreshToken(req.getRefreshToken(), now);

        if (result.isEmpty()) {
            logger.info("Refresh: {\"refresh_token\":\"{}\"} refresh_token 失效",
                    req.getRefreshToken());
            throw new ResponseStatusException(HttpStatus.CONFLICT, "132");
        }

        AuthDomain auth = result.get();
        UUID user_id = auth.getUser().getUser_id();
        String access_token = JWT.createToken(user_id);

        var user = auth.getUser();
        user.setLast_login_at(OffsetDateTime.now());

        logger.info("Refresh: {\"refresh_token\":\"{}\"} Refresh successful user_id: {}",
                req.getRefreshToken(),
                user_id);
        return RefreshResponse.builder()
                .accessToken(access_token)
                .build();
    }
}