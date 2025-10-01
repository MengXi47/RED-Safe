package com.redsafetw.edge_service.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.Optional;

import com.redsafetw.edge_service.domain.EdgeDeviceDomain;


/**
 * 邊緣裝置資料存取介面
 *
 * @create 2025-09-14
 */
@Repository
public interface EdgeRepository extends JpaRepository<EdgeDeviceDomain, String> {
    Optional<EdgeDeviceDomain> findByEdgeId(String edgeId);
    boolean existsByEdgeId(String edgeId);
}