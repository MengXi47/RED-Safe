package com.redsafetw.bind_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

/**
 * 綁定請求體
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
@Builder
public class UnBindRequest {
    @NotBlank(message = "125")
    @JsonProperty("edge_id")
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;
}
