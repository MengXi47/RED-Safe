package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Pattern;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class EdgeStatusReportRequest {

    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    @JsonProperty("online")
    @NotNull(message = "150")
    private Boolean online;
}
