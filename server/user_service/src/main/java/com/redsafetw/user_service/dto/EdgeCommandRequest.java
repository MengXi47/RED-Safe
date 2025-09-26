package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * Edge 指令請求資料
 */
@Getter
@Setter
public class EdgeCommandRequest {

    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    private String edgeId;

    @JsonProperty("code")
    @NotBlank(message = "149")
    private String code;
}
