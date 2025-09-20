package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

/**
 * 綁定請求體
 *
 * @create 2025-09-14
 **/
@Getter
@Setter
public class BindRequest {
    @NotBlank(message = "125")
    @JsonProperty("edge_id")
    private String edgeId;

    @NotBlank(message = "122")
    @JsonProperty("edge_name")
    private String edgeName;

    @NotBlank(message = "144")
    @JsonProperty("edge_password")
    private String edgePassword;

}
