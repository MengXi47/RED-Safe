package com.redsafetw.edge_service.grpc;

import com.grpc.fallinference.FallInferenceRequest;
import com.grpc.fallinference.FallInferenceResponse;
import com.grpc.fallinference.FallInferenceServiceGrpc;
import java.util.List;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class FallInferenceGrpcClient {

    private final FallInferenceServiceGrpc.FallInferenceServiceBlockingStub stub;

    public double inferFallProbability(List<Float> features) {
        FallInferenceRequest request = FallInferenceRequest.newBuilder()
                .addAllFeatures(features)
                .build();
        FallInferenceResponse response = stub.inferFallProbability(request);
        return response.getProbability();
    }
}
