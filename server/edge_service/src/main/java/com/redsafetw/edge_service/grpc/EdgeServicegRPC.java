package com.redsafetw.edge_service.grpc;

import com.grpc.edge.*;
import com.redsafetw.edge_service.repository.EdgeRepository;
import com.redsafetw.edge_service.service.EdgeVerify;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import org.springframework.grpc.server.service.GrpcService;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

/**
 * gRPC Edge服務
 *
 * @create 2025-09-14
 */
@GrpcService
@RequiredArgsConstructor
public class EdgeServicegRPC extends EdgeServiceGrpc.EdgeServiceImplBase {
    private final EdgeRepository edgeRepository;
    private final EdgeVerify edgeVerify;

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
    public void verifyEdgeCredentials(
            VerifyEdgeCredentialsRequest request,
            StreamObserver<VerifyEdgeCredentialsResponse> responseObserver) {
        boolean valid = edgeVerify.verifyCredentials(request.getEdgeId(), request.getPassword());

        VerifyEdgeCredentialsResponse reply = VerifyEdgeCredentialsResponse.newBuilder()
                .setValid(valid)
                .build();

        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }

    @Override
    public void updateEdgePassword(
            UpdateEdgePasswordRequest request,
            StreamObserver<UpdateEdgePasswordResponse> responseObserver) {
        try {
            edgeVerify.updatePassword(request.getEdgeId(), request.getEdgePassword(), request.getNewEdgePassword());

            UpdateEdgePasswordResponse reply = UpdateEdgePasswordResponse.newBuilder()
                    .setErrorCode("0")
                    .build();

            responseObserver.onNext(reply);
            responseObserver.onCompleted();
        } catch (ResponseStatusException ex) {
            String errorCode = mapToErrorCode(ex);
            UpdateEdgePasswordResponse reply = UpdateEdgePasswordResponse.newBuilder()
                    .setErrorCode(errorCode)
                    .build();
            responseObserver.onNext(reply);
            responseObserver.onCompleted();
        } catch (Exception ex) {
            responseObserver.onError(Status.INTERNAL.withDescription(ex.getMessage()).withCause(ex).asRuntimeException());
        }
    }

    private String mapToErrorCode(ResponseStatusException ex) {
        int statusCode = ex.getStatusCode().value();
        String reason = ex.getReason();

        if (statusCode == HttpStatus.NOT_FOUND.value()) {
            return "123";
        }
        if (statusCode == HttpStatus.UNAUTHORIZED.value()) {
            return "147";
        }
        if ("EDGE_UPDATE_PASSWORD_INVALID_REQUEST".equals(reason)) {
            return "400";
        }
        return "999";
    }
}
