package com.redsafetw.user_service.dto;

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
    private UUID user_id;
    private String user_name;
}