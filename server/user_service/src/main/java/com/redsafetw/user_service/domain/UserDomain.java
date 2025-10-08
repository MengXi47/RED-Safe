package com.redsafetw.user_service.domain;

import jakarta.persistence.*;

import java.time.OffsetDateTime;
import java.util.UUID;

import lombok.Getter;
import lombok.Setter;
import org.checkerframework.checker.units.qual.C;
import org.hibernate.annotations.UuidGenerator;

/**
 * @author boen
 * @create 2025-09-11 10:18 pm
 **/
@Setter
@Getter
@Entity
@Table(name = "users")
public class UserDomain {

    /**
     * 使用者的唯一識別碼
     **/
    @Id
    @UuidGenerator
    @Column(name = "user_id", nullable = false, updatable = false)
    private UUID userId;

    /**
     * 使用者電子郵件
     **/
    @Column(name = "email", nullable = false, unique = true)
    private String email;

    /**
     * 使用者名稱
     **/
    @Column(name = "user_name")
    private String user_name;

    /**
     * 使用者密碼的雜湊值
     **/
    @Column(name = "user_password_hash", nullable = false)
    private String user_password_hash;

    /**
     * 使用者帳號建立時間，資料庫預設為當前時間
     **/
    @Column(name = "created_at", nullable = false, insertable = false, updatable = false, columnDefinition = "TIMESTAMPTZ DEFAULT NOW()")
    private OffsetDateTime created_at;

    /**
     * 使用者最後登入時間
     */
    @Column(name = "last_login_at")
    private OffsetDateTime last_login_at;

    /**
     * 使用者的 TOTP 秘鑰（用於產生 30 秒一次的 OTP）
     */
    @Column(name = "otp_secret")
    private String otpSecret;

    /**
     * 是否啟用二階段驗證
     */
    @Column(name = "otp_enabled")
    private Boolean otpEnabled;

    /**
     * 備用驗證碼（陣列型別），萬一用戶手機丟失還能登入，三組 6 位數驗證碼
     */
    @Column(name = "otp_backup_codes", columnDefinition = "text[]")
    @org.hibernate.annotations.JdbcTypeCode(org.hibernate.type.SqlTypes.ARRAY)
    private String[] otpBackupCodes;
}
