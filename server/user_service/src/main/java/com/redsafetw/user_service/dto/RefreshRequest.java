package com.redsafetw.user_service.dto;

import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * JWT換發請求體
 *
 * @create 2025-09-12 3:11 pm
 **/
@Getter
@Setter
public class RefreshRequest {
    @NotBlank
    private String refresh_token;
}