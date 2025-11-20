package com.redsafetw.edge_service.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import java.time.OffsetDateTime;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class FallEventReportRequest {

    @JsonProperty("edge_id")
    @NotBlank(message = "125")
    @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120")
    private String edgeId;

    @JsonProperty("ip_address")
    @NotBlank(message = "ip_required")
    private String ipAddress;

    @JsonProperty("ipc_name")
    private String ipcName;

    private String location;

    @JsonProperty("event_time")
    private OffsetDateTime eventTime;

    @JsonProperty("snapshot_base64")
    private String snapshotBase64;

    @JsonProperty("snapshot_mime_type")
    private String snapshotMimeType;
}
