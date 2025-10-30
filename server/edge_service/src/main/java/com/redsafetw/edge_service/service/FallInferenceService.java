package com.redsafetw.edge_service.service;

import com.redsafetw.edge_service.dto.ErrorCodeResponse;
import com.redsafetw.edge_service.dto.FallInferenceRequestDto;
import com.redsafetw.edge_service.dto.FallInferenceRequestDto.WindowBatch;
import com.redsafetw.edge_service.dto.FallInferenceRequestDto.WindowFrame;
import com.redsafetw.edge_service.grpc.FallInferenceGrpcClient;
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

    public ErrorCodeResponse inferFall(FallInferenceRequestDto requestDto) {
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
            FallInferenceRequestDto requestDto,
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
                log.info(
                        "Fall inference result edge={} window={} batch={} frame={} probability={}",
                        requestDto.getEdgeId(),
                        windowLabel,
                        batchIndex,
                        frameIndex,
                        probability
                );
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
