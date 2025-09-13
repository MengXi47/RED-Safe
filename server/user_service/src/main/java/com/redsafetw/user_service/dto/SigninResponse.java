package com.redsafetw.user_service.dto;

import lombok.Getter;
import lombok.Setter;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import lombok.Builder;

/**
 * 使用者登入回覆
 *
 * @create 2025-09-12 3:11 pm
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class SigninResponse {

    private String user_name;
    /** JWT 訪問權杖（Bearer Token） */
    private String access_token;

    /** 用於換發新的 access token */
    private String refresh_token;

    /** 權杖類型，預設 Bearer */
    @Builder.Default
    private String tokenType = "Bearer";
}