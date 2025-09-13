package com.redsafetw.user_service.service;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.util.*;
import com.redsafetw.user_service.repository.AuthRepository;
import com.redsafetw.user_service.domain.AuthDomain;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
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

    public RefreshResponse refresh(RefreshRequest req) {

        OffsetDateTime now = OffsetDateTime.now();
        Optional<AuthDomain> result = authRepository.findActiveByRefreshToken(req.getRefresh_token(), now);

        if (result.isEmpty()) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "JWT失效");
        }

        AuthDomain auth = result.get();
        UUID user_id = auth.getUser().getUser_id();
        String access_token = JWT.createToken(user_id);

        return RefreshResponse.builder()
                .access_token(access_token)
                .build();
    }
}