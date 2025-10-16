package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

import java.util.List;

/**
 * 獲取已綁定的所有使用者回覆體
 *
 * @create 2025-10-14
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class EdgeUserBindListResponse {
    /**
     * 綁定使用者列表
     */
    @JsonProperty("users")
    private List<UserItem> users;

    @Getter
    @Setter
    @NoArgsConstructor
    @AllArgsConstructor
    @Builder
    public static class UserItem {
        @JsonProperty("user_id")
        private String userId;

        @JsonProperty("email")
        private String email;

        @JsonProperty("user_name")
        private String userName;

        @JsonProperty("bind_at")
        private String bindAt;

        @JsonProperty("last_online")
        private String lastOnline;
    }
}
