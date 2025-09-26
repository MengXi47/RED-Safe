package com.redsafetw.edge_service.controller;

import com.redsafetw.edge_service.dto.*;
import com.redsafetw.edge_service.service.*;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

/**
 * 邊緣裝置控制器
 *
 * @create 2025-09-14
 */
@Validated
@RestController
@RequiredArgsConstructor
@RequestMapping("/edge")
@Slf4j
public class EdgeController {
    private final EdgeRegisterService EdgeRegister;
    private final EdgeVerify edgeVerify;

    @PostMapping("/reg")
    public EdgeRegisterResponse registerEdge(@Valid @RequestBody EdgeRegisterRequest edgeRegisterRequest) {
        return EdgeRegister.registerEdge(edgeRegisterRequest);
    }

    @PostMapping("/update/edge_password")
    public ErrorCodeResponse updateEdgePassword(
            @Valid @RequestBody EdgePasswordUpdateRequest request) {
        edgeVerify.updatePassword(request.getEdgeId(), request.getEdgePassword(), request.getNewEdgePassword());
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    @PostMapping("/status/report")
    public ErrorCodeResponse reportStatus(@Valid @RequestBody EdgeStatusReportRequest request) {
        log.info("Edge status report received. edge_id={} online={}", request.getEdgeId(), request.getOnline());
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    @PostMapping("/ipcscan/report")
    public ErrorCodeResponse reportIpcscan(@Valid @RequestBody IpcscanReportRequest request) {
        String preview = request.getResult();
        if (preview.length() > 200) {
            preview = preview.substring(0, 200) + "...";
        }
        log.info("IPCscan report received. edge_id={} preview={} (length={})",
                request.getEdgeId(), preview, request.getResult().length());
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

}
