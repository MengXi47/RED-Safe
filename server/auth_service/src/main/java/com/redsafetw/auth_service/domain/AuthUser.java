package com.redsafetw.auth_service.domain;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.PrePersist;
import jakarta.persistence.PreUpdate;
import jakarta.persistence.Table;
import lombok.Getter;
import lombok.Setter;
import org.hibernate.annotations.JdbcTypeCode;
import org.hibernate.type.SqlTypes;

import java.time.OffsetDateTime;
import java.util.UUID;

@Getter
@Setter
@Entity
@Table(name = "auth_users")
public class AuthUser {

    @Id
    @Column(name = "user_id", nullable = false, updatable = false)
    private UUID userId;

    @Column(name = "email", nullable = false, unique = true)
    private String email;

    @Column(name = "email_is_verified", nullable = false)
    private Boolean emailVerified = Boolean.FALSE;

    @Column(name = "email_verified_at")
    private OffsetDateTime emailVerifiedAt;

    @Column(name = "password_hash", nullable = false)
    private String passwordHash;

    @Column(name = "created_at", nullable = false, insertable = false, updatable = false,
            columnDefinition = "TIMESTAMPTZ DEFAULT NOW()")
    private OffsetDateTime createdAt;

    @Column(name = "updated_at", nullable = false)
    private OffsetDateTime updatedAt;

    @Column(name = "last_login_at")
    private OffsetDateTime lastLoginAt;

    @Column(name = "last_password_at", nullable = false)
    private OffsetDateTime lastPasswordAt;

    @Column(name = "otp_secret")
    private String otpSecret;

    @Column(name = "otp_enabled", nullable = false)
    private Boolean otpEnabled = Boolean.FALSE;

    @JdbcTypeCode(SqlTypes.ARRAY)
    @Column(name = "otp_backup_codes", columnDefinition = "text[]")
    private String[] otpBackupCodes;

    @Column(name = "otp_enabled_at")
    private OffsetDateTime otpEnabledAt;

    @Column(name = "revoked", nullable = false)
    private Boolean revoked = Boolean.FALSE;

    @PrePersist
    public void onCreate() {
        OffsetDateTime now = OffsetDateTime.now();
        if (userId == null) {
            userId = UUID.randomUUID();
        }
        if (updatedAt == null) {
            updatedAt = now;
        }
        if (lastPasswordAt == null) {
            lastPasswordAt = now;
        }
    }

    @PreUpdate
    public void onUpdate() {
        updatedAt = OffsetDateTime.now();
    }
}
