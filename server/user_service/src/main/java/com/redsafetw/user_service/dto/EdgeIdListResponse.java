package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

import java.util.List;

/**
 * 獲取所有 edge_id 回覆
 *
 * @create 2025-09-12 3:11 pm
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class EdgeIdListResponse {
    /**
     * 所有綁定的邊緣裝置 edge ID 列表
     */
    @JsonProperty("edge_id")
    private List<String> edgeIds;
}