package com.redsafetw.user_service.dto;

import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * 使用者註冊請求
 *
 * @create 2025-09-11 10:40 pm
 **/
@Getter
@Setter
public class SignupRequest {
    @Email
    @NotBlank
    private String email;

    @NotBlank
    private String user_name;

    @NotBlank
    private String password;
}