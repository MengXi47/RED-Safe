package com.redsafetw.edge_service.domain;

import jakarta.persistence.*;
import java.time.OffsetDateTime;

import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * 邊緣裝置表
 * @create 2025-09-14
 */
@Getter
@Setter
@Entity
@Table(name = "edge_devices")
public class EdgeDeviceDomain {

    /**
     * 邊緣裝置ID
     */
    @Id
    @Column(name = "edge_id", length = 12, nullable = false, updatable = false)
    private String edgeId;


    /**
     * Edge裝置密碼的雜湊值
     **/
    @Column(name = "edge_password_hash", nullable = false)
    private String edgePasswordHash;

    /**
     * 邊緣裝置版本
     */
    @Column(name = "version", length = 32, nullable = false)
    private String version;

    /**
     * 邊緣裝置註冊時間
     */
    @Column(name = "registered_at", nullable = false, insertable = false, updatable = false,
            columnDefinition = "TIMESTAMPTZ DEFAULT NOW()")
    private OffsetDateTime registeredAt;

    /**
     * 邊緣裝置最後上線時間
     */
    @Column(name = "last_online_at")
    private OffsetDateTime lastOnlineAt;
}
