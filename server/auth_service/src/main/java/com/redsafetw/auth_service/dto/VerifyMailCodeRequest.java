package com.redsafetw.auth_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

import java.util.UUID;

@Getter
@Setter
public class VerifyMailCodeRequest {
    @NotBlank(message = "157")
    @JsonProperty("user_id")
    private String userId;

    @NotBlank(message = "156")
    private String code;
}
