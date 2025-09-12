package com.redsafetw.user_service.dto;

import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * 使用者登入請求
 *
 * @create 2025-09-12 3:11 pm
 **/
@Getter
@Setter
public class SigninRequest {
    @NotBlank
    @Email
    private String email;
    @NotBlank
    private String password;
}