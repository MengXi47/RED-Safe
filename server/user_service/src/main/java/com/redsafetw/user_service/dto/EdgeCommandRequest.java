package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.JsonNode;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
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
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    @JsonProperty("code")
    @NotBlank(message = "149")
    private String code;

    @JsonProperty("payload")
    private JsonNode payload;
}
