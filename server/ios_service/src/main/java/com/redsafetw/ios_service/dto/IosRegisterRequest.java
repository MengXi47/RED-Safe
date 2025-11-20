package com.redsafetw.ios_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.Getter;
import lombok.Setter;

import java.util.UUID;

/**
 * iOS 裝置註冊請求
 */
@Getter
@Setter
public class IosRegisterRequest {
    @JsonProperty("ios_device_id")
    private UUID iosDeviceId;

    @JsonProperty("apns_token")
    @NotBlank(message = "130")
    @Size(max = 512, message = "131")
    private String apnsToken;

    @JsonProperty("device_name")
    @Size(max = 255, message = "132")
    private String deviceName;
}
