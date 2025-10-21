package com.redsafetw.auth_service.dto;

import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class VerifyMailCodeRequest {

    @NotBlank(message = "156")
    private String code;
}
