package com.redsafetw.user_service.dto;

import lombok.*;

/**
 * JWT換發回覆
 *
 * @create 2025-09-12 3:11 pm
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class RefreshResponse {
    /** JWT 訪問權杖（Bearer Token） */
    private String access_token;
}