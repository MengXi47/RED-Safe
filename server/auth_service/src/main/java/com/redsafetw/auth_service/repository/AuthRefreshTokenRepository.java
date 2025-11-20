package com.redsafetw.auth_service.repository;

import com.redsafetw.auth_service.domain.AuthRefreshToken;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.OffsetDateTime;
import java.util.Optional;

@Repository
public interface AuthRefreshTokenRepository extends JpaRepository<AuthRefreshToken, String> {

    @Query("select a from AuthRefreshToken a where a.refreshToken = :token and a.revoked = false and a.expiresAt > :now")
    Optional<AuthRefreshToken> findActiveByRefreshToken(@Param("token") String token, @Param("now") OffsetDateTime now);
}
