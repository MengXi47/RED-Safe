package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotEmpty;
import jakarta.validation.constraints.Pattern;
import java.time.OffsetDateTime;
import java.util.List;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class FallInferenceRequest {

    @NotEmpty
    private List<@Valid WindowBatch> windows;

    @JsonProperty("ip_address")
    private String ipAddress;

    @JsonProperty("ipc_name")
    private String ipcName;

    @JsonProperty("fall_sensitivity")
    private double fallSensitivity;

    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    private OffsetDateTime time;

    @Getter
    @Setter
    public static class WindowBatch {
        private List<@Valid WindowFrame> window1;
        private List<@Valid WindowFrame> window2;
        private List<@Valid WindowFrame> window3;
    }

    @Getter
    @Setter
    public static class WindowFrame {
        private double a1;
        private double r1;
        private double h1;
        private double a2;
        private double r2;
        private double h2;
        private double a3;
        private double r3;
        private double h3;
    }
}
