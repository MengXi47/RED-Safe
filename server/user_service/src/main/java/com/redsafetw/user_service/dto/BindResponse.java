package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

/**
 * 綁定回覆
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class BindResponse {
    @JsonProperty("error_code")
    private String errorCode;
}