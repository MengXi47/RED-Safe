package com.redsafetw.edge_service.domain;

import jakarta.persistence.*;
import java.time.OffsetDateTime;
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
     * 邊緣裝置名稱
     **/
    @Column(name = "edge_name")
    private String edgeName;

    /**
     * 邊緣裝置版本
     */
    @Column(name = "version", length = 32, nullable = false)
    private String version;

    /**
     * 邊緣裝置註冊時間 RED-[0-9A-F]{8}
     */
    @Column(name = "registered_at", nullable = false, insertable = false, updatable = false,
            columnDefinition = "TIMESTAMPTZ DEFAULT NOW()")
    private OffsetDateTime registeredAt;
}