package com.redsafetw.bind_service.repository;

import com.redsafetw.bind_service.domain.UserEdgeBindDomain;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.UUID;

@Repository
public interface UserEdgeBindRepository extends
        JpaRepository<UserEdgeBindDomain, UserEdgeBindDomain.BindId> {
    List<UserEdgeBindDomain> findByUserId(UUID userId);
    List<UserEdgeBindDomain> findByEdgeId(String edgeId);
    boolean existsByUserIdAndEdgeId(UUID userId, String edgeId);
    void deleteByUserIdAndEdgeId(UUID userId, String edgeId);

}
