package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import lombok.Getter;
import lombok.Setter;

/**
 * 邊緣裝置註冊請求體
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
public class EdgeRegisterRequest {
    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    @JsonProperty("edge_password")
    @NotBlank(message = "144")
    private String edgePassword;

    @NotBlank(message = "145")
    @Pattern(regexp = "^\\d+\\.\\d+\\.\\d+$", message = "121")
    private String version;
}
