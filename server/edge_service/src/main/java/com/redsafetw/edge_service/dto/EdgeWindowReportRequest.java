package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * /edge/test 視窗資料回報 DTO
 */
@Getter
@Setter
@JsonIgnoreProperties(ignoreUnknown = true)
public class EdgeWindowReportRequest {

    private List<WindowGroup> windows;

    @JsonProperty("edge_id")
    private String edgeId;

    private String time;

    @Getter
    @Setter
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class WindowGroup {
        private List<WindowMetrics> window1;
        private List<WindowMetrics> window2;
        private List<WindowMetrics> window3;
    }

    @Getter
    @Setter
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class WindowMetrics {
        private Double a1;
        private Double r1;
        private Double h1;
        private Double a2;
        private Double r2;
        private Double h2;
        private Double a3;
        private Double r3;
        private Double h3;
    }
}
