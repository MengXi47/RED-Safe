package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class EdgeStatusReportRequest {

    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    private String edgeId;

    @JsonProperty("online")
    @NotNull(message = "150")
    private Boolean online;
}
