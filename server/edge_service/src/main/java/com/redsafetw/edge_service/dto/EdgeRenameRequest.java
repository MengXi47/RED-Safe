package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import lombok.Getter;
import lombok.Setter;

/**
 * 邊緣裝置重命名請求體
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
public class EdgeRenameRequest {
    @JsonProperty("edge_id")
    @NotBlank
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    @JsonProperty("edge_name")
    @NotBlank(message = "122")
    private String edgeName;
}