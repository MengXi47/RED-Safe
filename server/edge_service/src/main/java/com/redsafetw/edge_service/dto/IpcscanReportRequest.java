package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class IpcscanReportRequest {

    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    private String edgeId;

    @JsonProperty("result")
    @NotBlank(message = "151")
    private String result;
}
