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
     * 所有綁定的邊緣裝置資訊
     */
    @JsonProperty("edges")
    private List<EdgeItem> edges;

    @Getter
    @Setter
    @NoArgsConstructor
    @AllArgsConstructor
    @Builder
    public static class EdgeItem {
        @JsonProperty("edge_id")
        private String edgeId;

        @JsonProperty("display_name")
        private String displayName;

        @JsonProperty("is_online")
        private Boolean isOnline;
    }
}
