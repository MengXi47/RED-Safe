package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
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
    @NotBlank(message = "129")
    @Email
    @Pattern(regexp = "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$", message = "124")
    private String email;

    @NotBlank(message = "130")
    private String password;

    @JsonProperty("otp_code")
    private String otpCode;

    @JsonProperty("otp_back_code")
    private String backupCode;
}
