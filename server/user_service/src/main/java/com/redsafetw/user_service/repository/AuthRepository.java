package com.redsafetw.user_service.repository;

import com.redsafetw.user_service.domain.AuthDomain;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.OffsetDateTime;
import java.util.Optional;

/**
 * 認證系統存取介面
 *
 * @create 2025-09-12
 **/
@Repository
public interface AuthRepository extends JpaRepository<AuthDomain, String> {

    /** 依 refresh token 查詢「有效中的」紀錄（未撤銷且未過期） */
    @Query("select a from AuthDomain a where a.refresh_token = :token and a.revoked = false and a.expires_at > :now")
    Optional<AuthDomain> findActiveByRefreshToken(@Param("token") String token, @Param("now") OffsetDateTime now);
}