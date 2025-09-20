package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

/**
 * 統一回覆
 *
 * @create 2025-09-18
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class ErrorCodeResponse {
    @JsonProperty("error_code")
    private String errorCode;
}