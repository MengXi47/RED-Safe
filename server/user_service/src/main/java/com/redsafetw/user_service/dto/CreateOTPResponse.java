package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

import java.util.List;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class CreateOTPResponse {
    @JsonProperty("otp_key")
    String otpKey;

    @JsonProperty("backup_codes")
    List<String> backupCodes;
}
