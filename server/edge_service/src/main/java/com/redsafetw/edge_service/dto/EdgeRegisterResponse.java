package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.*;

/**
 * 邊緣裝置註冊回覆
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class EdgeRegisterResponse {
    @JsonProperty("edge_id")
    @NotBlank
    private String edgeId;
}