package com.redsafetw.user_service.domain;

import jakarta.persistence.*;
import java.time.OffsetDateTime;

import lombok.Getter;
import lombok.Setter;

/**
 * @author boen
 * @create 2025-09-12
 **/
@Setter
@Getter
@Entity
@Table(name = "auth")
public class AuthDomain {

    /**
     * refresh token 的雜湊值
     **/
    @Id
    @Column(name = "refresh_token", nullable = false, unique = true)
    private String refresh_token;

    /**
     * 關聯的使用者 ID
     **/
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "user_id", nullable = false)
    private UserDomain user;

    /**
     * token 建立時間
     **/
    @Column(name = "created_at", nullable = false, insertable = false, updatable = false, columnDefinition = "TIMESTAMPTZ DEFAULT NOW()")
    private OffsetDateTime created_at;

    /**
     * token 過期時間
     **/
    @Column(name = "expires_at", nullable = false)
    private OffsetDateTime expires_at;

    /**
     * 是否已撤銷
     **/
    @Column(name = "revoked", nullable = false)
    private Boolean revoked = false;
}