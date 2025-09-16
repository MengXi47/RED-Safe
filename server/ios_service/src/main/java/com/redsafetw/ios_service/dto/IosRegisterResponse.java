package com.redsafetw.ios_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.UUID;

/**
 * iOS 裝置註冊回應
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class IosRegisterResponse {
    @JsonProperty("ios_device_id")
    private UUID iosDeviceId;

    @JsonProperty("apns_token")
    private String apnsToken;

    @JsonProperty("device_name")
    private String deviceName;
}
