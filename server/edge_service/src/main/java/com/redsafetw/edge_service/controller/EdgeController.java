package com.redsafetw.edge_service.controller;

import com.redsafetw.edge_service.dto.*;
import com.redsafetw.edge_service.service.EdgeOnlineService;
import com.redsafetw.edge_service.service.EdgeRegisterService;
import com.redsafetw.edge_service.service.EdgeVerify;
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
    private final EdgeRegisterService edgeRegisterService;
    private final EdgeVerify edgeVerify;
    private final EdgeOnlineService edgeOnlineService;

    @PostMapping("/reg")
    public EdgeRegisterResponse registerEdge(@Valid @RequestBody EdgeRegisterRequest edgeRegisterRequest) {
        return edgeRegisterService.registerEdge(edgeRegisterRequest);
    }

    @PostMapping("/update/edge_password")
    public ErrorCodeResponse updateEdgePassword(
            @Valid @RequestBody EdgePasswordUpdateRequest request) {
        edgeVerify.updatePassword(request.getEdgeId(), request.getEdgePassword(), request.getNewEdgePassword());
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    @PostMapping("/online")
    public ErrorCodeResponse reportOnline(@Valid @RequestBody EdgeOnlineRequest request) {
        return edgeOnlineService.handleOnline(request);
    }
}
