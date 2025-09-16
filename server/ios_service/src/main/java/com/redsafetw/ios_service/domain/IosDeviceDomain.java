package com.redsafetw.ios_service.domain;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.PrePersist;
import jakarta.persistence.PreUpdate;
import jakarta.persistence.Table;
import lombok.Getter;
import lombok.Setter;

import java.time.OffsetDateTime;
import java.util.UUID;

/**
 * iOS 裝置資料表對應
 */
@Getter
@Setter
@Entity
@Table(name = "ios_devices")
public class IosDeviceDomain {

    @Id
    @GeneratedValue(strategy = GenerationType.UUID)
    @Column(name = "ios_device_id", nullable = false, updatable = false)
    private UUID iosDeviceId;

    @Column(name = "user_id", nullable = false)
    private UUID userId;

    @Column(name = "apns_token", nullable = false, unique = true)
    private String apnsToken;

    @Column(name = "device_name")
    private String deviceName;

    @Column(name = "last_seen_at", nullable = false)
    private OffsetDateTime lastSeenAt;

    @PrePersist
    @PreUpdate
    public void touchLastSeenAt() {
        this.lastSeenAt = OffsetDateTime.now();
    }
}
