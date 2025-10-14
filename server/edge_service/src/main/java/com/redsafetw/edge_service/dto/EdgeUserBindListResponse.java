package com.redsafetw.edge_service.dto;

import lombok.*;

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
    // email, user_name, bind_at, last_online
}
