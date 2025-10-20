package com.redsafetw.auth_service.service;

import com.redsafetw.auth_service.util.JwtService;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;

@Component
public class AccessTokenManager {

    public String createAccessToken(UUID userId) {
        return JwtService.createToken(userId);
    }

    public UUID verifyAccessToken(String token) {
        return JwtService.verifyAndGetUserId(token);
    }

    public UUID requireValidUserId(String token) {
        UUID userId = verifyAccessToken(token);
        if (userId.equals(new UUID(0L, 0L))) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }
        return userId;
    }
}
