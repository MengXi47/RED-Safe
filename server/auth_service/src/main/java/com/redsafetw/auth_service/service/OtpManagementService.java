package com.redsafetw.auth_service.service;

import com.redsafetw.auth_service.domain.AuthUser;
import com.redsafetw.auth_service.dto.CreateOTPResponse;
import com.redsafetw.auth_service.dto.ErrorCodeResponse;
import com.redsafetw.auth_service.repository.AuthUserRepository;
import com.warrenstrange.googleauth.GoogleAuthenticator;
import com.warrenstrange.googleauth.GoogleAuthenticatorConfig;
import com.warrenstrange.googleauth.GoogleAuthenticatorKey;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.security.SecureRandom;
import java.time.OffsetDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

@Component
@Slf4j
@RequiredArgsConstructor
public class OtpManagementService {

    private static final SecureRandom RANDOM = new SecureRandom();

    private final AuthUserRepository authUserRepository;
    private final GoogleAuthenticator googleAuthenticator = new GoogleAuthenticator(
            new GoogleAuthenticatorConfig.GoogleAuthenticatorConfigBuilder()
                    .setSecretBits(160)
                    .setCodeDigits(6)
                    .setTimeStepSizeInMillis(30_000)
                    .setWindowSize(1)
                    .build()
    );

    public CreateOTPResponse enableOtp(UUID userId) {
        AuthUser user = authUserRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        if (Boolean.TRUE.equals(user.getOtpEnabled()) && user.getOtpSecret() != null) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "153");
        }

        GoogleAuthenticatorKey key = googleAuthenticator.createCredentials();
        String otpSecret = key.getKey();
        List<String> backupCodes = generateBackupCodes(3);

        user.setOtpSecret(otpSecret);
        user.setOtpEnabled(Boolean.TRUE);
        user.setOtpBackupCodes(backupCodes.toArray(new String[0]));
        user.setOtpEnabledAt(OffsetDateTime.now());
        authUserRepository.save(user);

        return CreateOTPResponse.builder()
                .otpKey(otpSecret)
                .backupCodes(List.copyOf(backupCodes))
                .build();
    }

    public ErrorCodeResponse disableOtp(UUID userId) {
        AuthUser user = authUserRepository.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        if (!Boolean.TRUE.equals(user.getOtpEnabled()) || user.getOtpSecret() == null) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "154");
        }

        user.setOtpSecret(null);
        user.setOtpEnabled(Boolean.FALSE);
        user.setOtpBackupCodes(null);
        user.setOtpEnabledAt(null);
        authUserRepository.save(user);

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    public boolean verifyOtpOrBackup(AuthUser user, String rawCode) {
        String normalized = normalizeOtp(rawCode);
        if (normalized == null) {
            return false;
        }
        return verifyTotp(user.getOtpSecret(), normalized) || verifyBackupCode(user, normalized);
    }

    private boolean verifyTotp(String secret, String otpCode) {
        if (secret == null) {
            return false;
        }
        String normalized = normalizeOtp(otpCode);
        if (normalized == null) {
            return false;
        }
        try {
            int code = Integer.parseInt(normalized);
            return googleAuthenticator.authorize(secret, code);
        } catch (NumberFormatException ex) {
            log.warn("verifyTotp invalid format code={}", otpCode);
            return false;
        }
    }

    private boolean verifyBackupCode(AuthUser user, String backupCode) {
        String normalized = normalizeOtp(backupCode);
        if (normalized == null) {
            return false;
        }

        String[] codes = user.getOtpBackupCodes();
        if (codes == null || codes.length == 0) {
            return false;
        }

        for (int i = 0; i < codes.length; i++) {
            if (normalized.equals(codes[i])) {
                codes[i] = null;
                user.setOtpBackupCodes(codes);
                authUserRepository.save(user);
                return true;
            }
        }
        return false;
    }

    private List<String> generateBackupCodes(int count) {
        List<String> codes = new ArrayList<>(count);
        for (int i = 0; i < count; i++) {
            codes.add(String.format("%06d", RANDOM.nextInt(1_000_000)));
        }
        return codes;
    }

    private String normalizeOtp(String code) {
        if (code == null) {
            return null;
        }
        String sanitized = code.replace(" ", "").trim();
        return sanitized.isEmpty() ? null : sanitized;
    }
}
