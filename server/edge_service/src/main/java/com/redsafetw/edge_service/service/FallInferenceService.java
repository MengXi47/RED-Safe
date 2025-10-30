package com.redsafetw.edge_service.service;

import com.grpc.notify.NotifyService;
import com.grpc.user.ListEdgeUsersResponse;
import com.redsafetw.edge_service.dto.EdgeUserBindListResponse;
import com.redsafetw.edge_service.dto.ErrorCodeResponse;
import com.redsafetw.edge_service.dto.FallInferenceRequest;
import com.redsafetw.edge_service.dto.FallInferenceRequest.WindowBatch;
import com.redsafetw.edge_service.dto.FallInferenceRequest.WindowFrame;
import com.redsafetw.edge_service.grpc.FallInferenceGrpcClient;
import com.redsafetw.edge_service.grpc.NotifyGrpcClient;
import com.redsafetw.edge_service.grpc.UserGrpcClient;
import io.grpc.StatusRuntimeException;

import java.util.List;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

@Service
@RequiredArgsConstructor
@Slf4j
public class FallInferenceService {

    private final FallInferenceGrpcClient fallInferenceGrpcClient;

    private final UserGrpcClient userGrpcClient;
    private final NotifyGrpcClient notifySGrpcClient;


    public ErrorCodeResponse inferFall(FallInferenceRequest requestDto) {
        List<WindowBatch> windowBatches = requestDto.getWindows();
        if (windowBatches == null || windowBatches.isEmpty()) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "windows_required");
        }

        for (int batchIndex = 0; batchIndex < windowBatches.size(); batchIndex++) {
            WindowBatch batch = windowBatches.get(batchIndex);
            processWindowFrames(requestDto, batch.getWindow1(), "window1", batchIndex);
            processWindowFrames(requestDto, batch.getWindow2(), "window2", batchIndex);
            processWindowFrames(requestDto, batch.getWindow3(), "window3", batchIndex);
        }

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    private void processWindowFrames(
            FallInferenceRequest requestDto,
            List<WindowFrame> frames,
            String windowLabel,
            int batchIndex
    ) {
        if (frames == null || frames.isEmpty()) {
            return;
        }

        for (int frameIndex = 0; frameIndex < frames.size(); frameIndex++) {
            WindowFrame frame = frames.get(frameIndex);
            try {
                double probability = fallInferenceGrpcClient.inferFallProbability(toFeatureVector(frame));

                if ((probability * 100) <= requestDto.getFallSensitivity()) {
                    continue;
                }

                ListEdgeUsersResponse grpcResponse;
                grpcResponse = userGrpcClient.listEdgeUsers(requestDto.getEdgeId());

                List<EdgeUserBindListResponse.UserItem> users = grpcResponse.getUsersList().stream()
                        .map(user -> EdgeUserBindListResponse.UserItem.builder()
                                .userId(user.getUserId())
                                .email(user.getEmail())
                                .build())
                        .toList();

                for (EdgeUserBindListResponse.UserItem user : users) {
                    notifySGrpcClient.sendFallAlertEmail(user.getEmail(), requestDto.getEdgeId(), requestDto.getIpAddress(), requestDto.getIpcName());
                }

            } catch (StatusRuntimeException ex) {
                log.error(
                        "Fall inference gRPC call failed for edge {} window {} batch {} frame {}",
                        requestDto.getEdgeId(),
                        windowLabel,
                        batchIndex,
                        frameIndex,
                        ex
                );
                throw new ResponseStatusException(HttpStatus.BAD_GATEWAY, "fall_inference_failed");
            }
        }
    }

    private List<Float> toFeatureVector(WindowFrame frame) {
        return List.of(
                (float) frame.getA1(),
                (float) frame.getR1(),
                (float) frame.getH1(),
                (float) frame.getA2(),
                (float) frame.getR2(),
                (float) frame.getH2(),
                (float) frame.getA3(),
                (float) frame.getR3(),
                (float) frame.getH3()
        );
    }
}
