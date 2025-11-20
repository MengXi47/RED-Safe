package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.*;

/**
 * 解綁回覆
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class UnbindResponse {
    @JsonProperty("error_code")
    private String errorCode;
}