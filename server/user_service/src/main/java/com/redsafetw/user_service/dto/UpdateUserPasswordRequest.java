package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * 更新使用者密碼請求體
 *
 * @create 2025-09-18
 **/
@Getter
@Setter
public class UpdateUserPasswordRequest {
    @NotBlank(message = "130")
    private String password;

    @JsonProperty("new_password")
    @NotBlank(message = "141")
    private String newPassword;
}