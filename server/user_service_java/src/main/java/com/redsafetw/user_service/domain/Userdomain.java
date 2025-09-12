package com.redsafetw.user_service.domain;

import jakarta.persistence.*;

import java.time.OffsetDateTime;
import java.util.UUID;

import lombok.Getter;
import lombok.Setter;
import org.hibernate.annotations.UuidGenerator;

/**
 * @author boen
 * @create 2025-09-11 10:18 pm
 **/
@Setter
@Getter
@Entity
@Table(name = "users")
public class Userdomain {

    /**
     * 使用者的唯一識別碼
     */
    @Id
    @UuidGenerator
    @Column(name = "user_id", nullable = false, updatable = false)
    private UUID user_id;

    /**
     * 使用者電子郵件
     */
    @Column(name = "email", nullable = false, unique = true)
    private String email;

    /**
     * 使用者名稱
     */
    @Column(name = "user_name")
    private String user_name;

    /**
     * 使用者密碼的雜湊值
     */
    @Column(name = "password", nullable = false)
    private String password_hash;

    /**
     * 使用者帳號建立時間，資料庫預設為當前時間
     */
    @Column(name = "created_at", nullable = false, insertable = false, updatable = false, columnDefinition = "TIMESTAMPTZ DEFAULT NOW()")
    private OffsetDateTime created_at;

    /**
     * 使用者最後登入時間
     */
    @Column(name = "login_at")
    private OffsetDateTime login_at;

}