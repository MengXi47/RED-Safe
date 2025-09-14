package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
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

    @JsonProperty("user_name")
    private String userName;

    @JsonProperty("access_token")
    private String accessToken;

    @JsonProperty("refresh_token")
    private String refreshToken;
}