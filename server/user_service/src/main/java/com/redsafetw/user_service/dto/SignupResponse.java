package com.redsafetw.user_service.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.UUID;

/**
 * 使用者註冊回覆
 *
 * @create 2025-09-11 10:40 pm
 **/
@Getter
@Setter
public class SignupResponse {
    private UUID user_id;
    private String user_name;

    public SignupResponse(UUID user_id, String username) {
        this.user_id = user_id;
        this.user_name = username;
    }
}