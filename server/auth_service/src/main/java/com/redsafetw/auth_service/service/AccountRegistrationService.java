package com.redsafetw.auth_service.service;

import com.redsafetw.auth_service.domain.AuthUser;
import com.redsafetw.auth_service.dto.SignupRequest;
import com.redsafetw.auth_service.dto.SignupResponse;
import com.redsafetw.auth_service.repository.AuthUserRepository;
import com.redsafetw.auth_service.grpc.UserGrpcClient;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.util.UUID;

@Component
@RequiredArgsConstructor
@Slf4j
public class AccountRegistrationService {

    private final AuthUserRepository authUserRepository;
    private final PasswordManagementService passwordManagementService;
    private final UserGrpcClient userGrpcClient;

    public SignupResponse signup(SignupRequest request) {
        String email = request.getEmail().trim().toLowerCase();
        if (authUserRepository.existsByEmail(email)) {
            log.info("Signup conflict email={}", email);
            throw new ResponseStatusException(HttpStatus.CONFLICT, "133");
        }
        UUID userId = UUID.randomUUID();
        AuthUser user = new AuthUser();
        user.setUserId(userId);
        user.setEmail(email);
        user.setPasswordHash(passwordManagementService.hashPassword(request.getPassword()));
        user.setEmailVerified(Boolean.FALSE);
        user.setOtpEnabled(Boolean.FALSE);
        authUserRepository.save(user);
        userGrpcClient.createUserProfile(userId, email, request.getUserName());
        log.info("Signup success user_id={} email={}", userId, email);
        return SignupResponse.builder()
                .userId(userId)
                .userName(request.getUserName())
                .build();
    }
}
