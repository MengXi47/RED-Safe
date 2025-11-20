package com.redsafetw.user_service.repository;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;
import java.util.UUID;

import com.redsafetw.user_service.domain.UserDomain;
import org.springframework.stereotype.Repository;

/**
 * 使用者資料存取介面
 *
 * @create 2025-09-11 11:20 pm
 **/
@Repository
public interface UserRepository extends JpaRepository<UserDomain, UUID> {
    Optional<UserDomain> findByEmail(String email);

    Optional<UserDomain> findByUserId(UUID userId);

    boolean existsByEmail(String email);
}