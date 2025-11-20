package com.redsafetw.auth_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class CreateOTPResponse {

    @JsonProperty("otp_key")
    private String otpKey;

    @JsonProperty("backup_codes")
    private List<String> backupCodes;
}
