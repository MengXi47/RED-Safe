package com.redsafetw.auth_service.service;

import com.redsafetw.auth_service.domain.AuthUser;
import com.redsafetw.auth_service.dto.ErrorCodeResponse;
import com.redsafetw.auth_service.grpc.NotifyGrpcClient;
import com.redsafetw.auth_service.repository.AuthUserRepository;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.server.ResponseStatusException;

import java.security.SecureRandom;
import java.time.Duration;
import java.time.OffsetDateTime;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

@Service
@RequiredArgsConstructor
@Slf4j
public class MailVerificationService {

    private static final Duration CODE_TTL = Duration.ofMinutes(10);
    private static final SecureRandom RANDOM = new SecureRandom();

    private final AuthUserRepository authUserRepository;
    private final StringRedisTemplate redisTemplate;
    private final NotifyGrpcClient notifyGrpcClient;

    public ErrorCodeResponse sendMailVerification(UUID userId) {
        AuthUser user = authUserRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        String email = user.getEmail();

        log.info("Preparing mail verification code user_id={} email={}", userId, email);

        String code = generateCode();
        String key = redisKey(userId);

        redisTemplate.opsForValue().set(key, code, CODE_TTL.toSeconds(), TimeUnit.SECONDS);
        try {
            notifyGrpcClient.sendMailVerification(email, code, (int) CODE_TTL.toMinutes());
            log.info("Mail verification code dispatched user_id={}", userId);
        } catch (Exception ex) {
            redisTemplate.delete(key);
            throw ex;
        }

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    @Transactional
    public ErrorCodeResponse verifyMailCode(UUID userId, String rawCode) {
        AuthUser user = authUserRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        String code = normalize(rawCode);
        if (code == null) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "156");
        }

        String key = redisKey(userId);
        String stored = redisTemplate.opsForValue().get(key);
        if (stored == null || !stored.equals(code)) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "156");
        }

        user.setEmailVerified(Boolean.TRUE);
        user.setEmailVerifiedAt(OffsetDateTime.now());
        authUserRepository.save(user);
        redisTemplate.delete(key);

        log.info("Mail verification success user_id={}", userId);

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    private String generateCode() {
        return String.format("%06d", RANDOM.nextInt(1_000_000));
    }

    private String normalize(String code) {
        if (code == null) {
            return null;
        }
        String trimmed = code.replace(" ", "").trim();
        return trimmed.isEmpty() ? null : trimmed;
    }

    private String redisKey(UUID userId) {
        return "mail:Verify:" + userId;
    }
}
