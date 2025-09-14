package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

import java.util.UUID;

/**
 * 使用者註冊回覆
 *
 * @create 2025-09-11 10:40 pm
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class SignupResponse {

    @JsonProperty("user_id")
    private UUID userId;

    @JsonProperty("user_name")
    private String userName;
}