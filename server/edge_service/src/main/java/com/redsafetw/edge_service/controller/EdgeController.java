package com.redsafetw.edge_service.controller;

import com.grpc.fall.FallInferenceRequest;
import com.grpc.fall.FallInferenceResponse;
import com.grpc.fall.FallInferenceServiceGrpc;
import com.redsafetw.edge_service.dto.*;
import io.grpc.StatusRuntimeException;
import com.redsafetw.edge_service.service.EdgeOnlineService;
import com.redsafetw.edge_service.service.EdgeRegisterService;
import com.redsafetw.edge_service.service.EdgeUserBindService;
import com.redsafetw.edge_service.service.EdgeVerify;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import java.util.List;
import java.util.Objects;
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
    private final EdgeUserBindService edgeUserBindService;
    private final FallInferenceServiceGrpc.FallInferenceServiceBlockingStub fallInferenceStub;

    @PostMapping("/reg")
    public ErrorCodeResponse registerEdge(@Valid @RequestBody EdgeRegisterRequest edgeRegisterRequest) {
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

    @GetMapping("/user/list")
    public EdgeUserBindListResponse getUserList(
            @RequestParam("edge_id")
            @NotBlank(message = "125")
            @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120") String edgeId) {
        return edgeUserBindService.getUserList(edgeId);
    }

    @PostMapping("/user/unbind")
    public ErrorCodeResponse unbindUser(@Valid @RequestBody EdgeUserUnbindRequest request) {
        return edgeUserBindService.unbindUser(request.getEdgeId(), null, request.getEmail());
    }

    @PostMapping("/test")
    public ErrorCodeResponse test(@RequestBody EdgeWindowReportRequest request) {
        if (request.getWindows() != null) {
            for (int groupIndex = 0; groupIndex < request.getWindows().size(); groupIndex++) {
                EdgeWindowReportRequest.WindowGroup group = request.getWindows().get(groupIndex);
                if (group == null) {
                    continue;
                }
                sendWindowMetrics(group.getWindow1(), "window1", groupIndex + 1);
                sendWindowMetrics(group.getWindow2(), "window2", groupIndex + 1);
                sendWindowMetrics(group.getWindow3(), "window3", groupIndex + 1);
            }
        }
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    private void sendWindowMetrics(
            List<EdgeWindowReportRequest.WindowMetrics> metricsList,
            String windowName,
            int groupIndex) {
        if (metricsList == null || metricsList.isEmpty()) {
            return;
        }
        for (int idx = 0; idx < metricsList.size(); idx++) {
            EdgeWindowReportRequest.WindowMetrics metrics = metricsList.get(idx);
            if (metrics == null) {
                continue;
            }
            List<Double> values = List.of(
                    metrics.getA1(), metrics.getR1(), metrics.getH1(),
                    metrics.getA2(), metrics.getR2(), metrics.getH2(),
                    metrics.getA3(), metrics.getR3(), metrics.getH3()
            );
            if (values.stream().anyMatch(Objects::isNull)) {
                log.warn("[gRPC] Skip {} group {} sample {} due to null feature values", windowName, groupIndex, idx + 1);
                continue;
            }
            FallInferenceRequest grpcRequest = FallInferenceRequest.newBuilder()
                    .addAllFeatures(values.stream().map(Double::floatValue).toList())
                    .build();
            try {
                FallInferenceResponse response = fallInferenceStub.inferFallProbability(grpcRequest);
                log.info(
                        "[gRPC] Fall inference {} group {} sample {} => {}%",
                        windowName,
                        groupIndex,
                        idx + 1,
                        response.getProbability()
                );
            } catch (StatusRuntimeException ex) {
                log.error(
                        "[gRPC] Fall inference failed for {} group {} sample {}: {}",
                        windowName,
                        groupIndex,
                        idx + 1,
                        ex.getStatus(),
                        ex
                );
            }
        }
    }
}
