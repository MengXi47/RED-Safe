package com.redsafetw.ios_service.repository;

import com.redsafetw.ios_service.domain.IosDeviceDomain;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.Optional;
import java.util.UUID;

/**
 * iOS 裝置資料存取介面
 */
@Repository
public interface IosDeviceRepository extends JpaRepository<IosDeviceDomain, UUID> {
    Optional<IosDeviceDomain> findByIosDeviceIdAndUserId(UUID iosDeviceId, UUID userId);

    Optional<IosDeviceDomain> findByApnsToken(String apnsToken);
}
