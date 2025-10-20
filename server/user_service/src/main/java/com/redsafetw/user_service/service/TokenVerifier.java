package com.redsafetw.user_service.service;

import com.redsafetw.user_service.grpc.AuthGrpcClient;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;

@Component
@RequiredArgsConstructor
public class TokenVerifier {

    private static final Logger log = LoggerFactory.getLogger(TokenVerifier.class);

    private final AuthGrpcClient authGrpcClient;

    public UUID requireUserId(String accessToken) {
        UUID userId = authGrpcClient.resolveUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            log.debug("Token invalid");
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }
        return userId;
    }
}
