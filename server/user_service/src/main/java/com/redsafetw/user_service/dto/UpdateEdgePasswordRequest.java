package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import lombok.Getter;
import lombok.Setter;

/**
 * 邊緣裝置密碼更新請求體
 */
@Getter
@Setter
public class UpdateEdgePasswordRequest {
    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    @JsonProperty("edge_password")
    @NotBlank(message = "144")
    private String edgePassword;

    @JsonProperty("new_edge_password")
    @NotBlank(message = "148")
    private String newEdgePassword;
}
