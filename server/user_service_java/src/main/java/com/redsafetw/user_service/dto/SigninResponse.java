package com.redsafetw.user_service.dto;

import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import lombok.Builder;
import java.time.OffsetDateTime;

import java.util.UUID;

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
    private UUID user_id;
    private String user_name;
    /** JWT 訪問權杖（Bearer Token） */
    private String access_token;

    /** 用於換發新的 access token */
    private String refresh_token;

    /** access token 過期時間（伺服器產生） */
    private OffsetDateTime accessTokenExpiresAt;

    /** 權杖類型，預設 Bearer */
    @Builder.Default
    private String tokenType = "Bearer";
}