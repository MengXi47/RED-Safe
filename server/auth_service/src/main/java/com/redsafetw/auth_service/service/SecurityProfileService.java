package com.redsafetw.auth_service.service;

import com.redsafetw.auth_service.domain.AuthUser;
import com.redsafetw.auth_service.repository.AuthUserRepository;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Optional;
import java.util.UUID;

@Component
@RequiredArgsConstructor
public class SecurityProfileService {

    private static final DateTimeFormatter ISO_FORMATTER = DateTimeFormatter.ISO_OFFSET_DATE_TIME;

    private final AuthUserRepository authUserRepository;

    public Optional<SecurityProfile> getSecurityProfile(UUID userId) {
        return authUserRepository.findByUserId(userId)
                .map(user -> new SecurityProfile(
                        Boolean.TRUE.equals(user.getOtpEnabled()),
                        user.getLastLoginAt(),
                        Boolean.TRUE.equals(user.getEmailVerified())
                ));
    }

    public record SecurityProfile(boolean otpEnabled, OffsetDateTime lastLoginAt, boolean emailVerified) {
        public String lastLoginIso() {
            return lastLoginAt == null ? "" : ISO_FORMATTER.format(lastLoginAt);
        }
    }
}
