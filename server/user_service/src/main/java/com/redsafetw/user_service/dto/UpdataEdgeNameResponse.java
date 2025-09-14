package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

/**
 * 邊緣裝置重命名回覆
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class UpdataEdgeNameResponse {
    @JsonProperty("error_code")
    private String errorCode;
}