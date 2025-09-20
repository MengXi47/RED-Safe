package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * 邊緣裝置密碼更新請求體
 */
@Getter
@Setter
public class EdgePasswordUpdateRequest {
    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    private String edgeId;

    @JsonProperty("edge_password")
    @NotBlank(message = "144")
    private String edgePassword;

    @JsonProperty("new_edge_password")
    @NotBlank(message = "148")
    private String newEdgePassword;
}
