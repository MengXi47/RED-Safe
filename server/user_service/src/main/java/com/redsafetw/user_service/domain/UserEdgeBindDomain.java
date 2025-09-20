package com.redsafetw.user_service.domain;

import jakarta.persistence.*;

import lombok.Getter;
import lombok.Setter;

import java.util.UUID;

/**
 * @author boen
 * @create 2025-09-14
 **/
@Setter
@Getter
@Entity
@Table(name = "user_edge_bind")
@IdClass(UserEdgeBindDomain.BindId.class)
public class UserEdgeBindDomain {

    // 邊緣裝置 ID
    @Id
    @Column(name = "edge_id", nullable = false, length = 12)
    private String edgeId;

    // 使用者 ID
    @Id
    @Column(name = "user_id", nullable = false)
    private UUID userId;

    @Column(name = "display_name")
    private String displayName;

    // JPA 複合主鍵類別
    @Getter
    @Setter
    @lombok.EqualsAndHashCode
    public static class BindId implements java.io.Serializable {
        // 邊緣裝置 ID（複合主鍵的一部分）
        private String edgeId;
        // 使用者 ID（複合主鍵的一部分）
        private UUID userId;
    }
}