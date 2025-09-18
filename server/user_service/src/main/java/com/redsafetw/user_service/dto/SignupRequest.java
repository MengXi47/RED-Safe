package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;
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
    @NotBlank(message = "129")
    @Pattern(regexp = "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$", message = "124")
    private String email;

    @NotBlank(message = "131")
    @JsonProperty("user_name")
    @Size(max = 16, message = "139")
    private String userName;

    @NotBlank(message = "130")
    private String password;
}