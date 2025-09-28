package com.redsafetw.user_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Builder;
import lombok.Value;

@Value
@Builder
public class EdgeCommandResponse {

    @JsonProperty("trace_id")
    String traceId;
}

