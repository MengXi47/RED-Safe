package com.redsafetw.auth_service.service;

import com.redsafetw.auth_service.domain.AuthUser;
import com.redsafetw.auth_service.repository.AuthUserRepository;
import com.redsafetw.auth_service.util.Argon2id;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.time.OffsetDateTime;
import java.util.Optional;
import java.util.UUID;

@Slf4j
@Component
@RequiredArgsConstructor
public class PasswordManagementService {

    private final AuthUserRepository authUserRepository;

    public String hashPassword(CharSequence rawValue) {
        return Argon2id.hash(rawValue);
    }

    public boolean verifyPassword(AuthUser user, String rawPassword) {
        if (user == null || rawPassword == null) {
            return false;
        }
        return Argon2id.verify(user.getPasswordHash(), rawPassword);
    }

    public boolean verifyPassword(UUID userId, String rawPassword) {
        if (userId == null || rawPassword == null) {
            return false;
        }
        Optional<AuthUser> userOpt = authUserRepository.findByUserId(userId);
        return userOpt.filter(user -> verifyPassword(user, rawPassword)).isPresent();
    }

    public PasswordChangeResult changePassword(UUID userId, String currentPassword, String newPassword) {
        if (userId == null) {
            return PasswordChangeResult.error("142");
        }

        AuthUser user = authUserRepository.findByUserId(userId)
                .orElse(null);
        if (user == null) {
            return PasswordChangeResult.error("142");
        }

        if (!verifyPassword(user, currentPassword)) {
            return PasswordChangeResult.error("143");
        }

        user.setPasswordHash(hashPassword(newPassword));
        user.setLastPasswordAt(OffsetDateTime.now());
        authUserRepository.save(user);

        return PasswordChangeResult.ok();
    }

    public record PasswordChangeResult(boolean success, String errorCode) {
        public static PasswordChangeResult ok() {
            return new PasswordChangeResult(true, "0");
        }

        public static PasswordChangeResult error(String errorCode) {
            return new PasswordChangeResult(false, errorCode);
        }
    }
}
