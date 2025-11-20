package com.redsafetw.auth_service.repository;

import com.redsafetw.auth_service.domain.AuthUser;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.Optional;
import java.util.UUID;

@Repository
public interface AuthUserRepository extends JpaRepository<AuthUser, UUID> {

    Optional<AuthUser> findByEmail(String email);

    Optional<AuthUser> findByUserId(UUID userId);

    boolean existsByEmail(String email);
}
