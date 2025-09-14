package com.redsafetw.edge_service.grpc;

import com.redsafetw.edge_service.repository.EdgeRepository;
import io.grpc.stub.StreamObserver;
import jakarta.validation.ConstraintViolation;
import jakarta.validation.Validator;
import lombok.RequiredArgsConstructor;
import org.springframework.grpc.server.service.GrpcService;
import com.grpc.edge.*;
import com.redsafetw.edge_service.service.EdgeRenameService;
import com.redsafetw.edge_service.dto.EdgeRenameRequest;

import java.util.Set;

/**
 * gRPC Edge服務
 *
 * @create 2025-09-14
 */
@GrpcService
@RequiredArgsConstructor
public class EdgeServicegRPC extends EdgeServiceGrpc.EdgeServiceImplBase {
    private final EdgeRepository edgeRepository;
    private final EdgeRenameService edgeRenameService;
    private final Validator validator;

    @Override
    public void checkEdgeIdExists(
            CheckEdgeIdExistsRequest request,
            StreamObserver<CheckEdgeIdExistsResponse> responseStreamObserver) {
        String edgeId = request.getEdgeId();

        boolean edgeIdExists = edgeRepository.existsById(edgeId); // 存在 -> TRUE

        CheckEdgeIdExistsResponse reply = CheckEdgeIdExistsResponse.newBuilder()
                .setExists(edgeIdExists).build();

        responseStreamObserver.onNext(reply);
        responseStreamObserver.onCompleted();
    }

    @Override
    public void updataEdgeName(
            UpdataEdgeNameRequest request,
            StreamObserver<UpdataEdgeNameResponse> responseStreamObserver) {

        EdgeRenameRequest req = new EdgeRenameRequest();
        req.setEdgeId(request.getEdgeId());
        req.setEdgeName(request.getEdgeName());

        Set<ConstraintViolation<EdgeRenameRequest>> violations = validator.validate(req);
        if (!violations.isEmpty()) {
            String errorCode = violations.iterator().next().getMessage(); // 120 或 122
            UpdataEdgeNameResponse reply = UpdataEdgeNameResponse.newBuilder()
                    .setErrorCode(errorCode)
                    .build();
            responseStreamObserver.onNext(reply);
            responseStreamObserver.onCompleted();
            return;
        }

        boolean result = edgeRenameService.rename(req);

        String errorCode = result ? "0" : "123";
        UpdataEdgeNameResponse reply = UpdataEdgeNameResponse.newBuilder()
                .setErrorCode(errorCode)
                .build();

        responseStreamObserver.onNext(reply);
        responseStreamObserver.onCompleted();
    }
}