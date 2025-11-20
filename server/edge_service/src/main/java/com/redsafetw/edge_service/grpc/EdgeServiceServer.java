package com.redsafetw.edge_service.grpc;

import com.grpc.edge.*;
import com.redsafetw.edge_service.repository.EdgeRepository;
import com.redsafetw.edge_service.service.EdgeVerify;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
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
@Slf4j
public class EdgeServiceServer extends EdgeServiceGrpc.EdgeServiceImplBase {
    private final EdgeRepository edgeRepository;
    private final EdgeVerify edgeVerify;

    @Override
    public void checkEdgeIdExists(
            CheckEdgeIdExistsRequest request,
            StreamObserver<CheckEdgeIdExistsResponse> responseStreamObserver) {
        String edgeId = request.getEdgeId();
        log.info("gRPC EdgeService.CheckEdgeIdExists edge_id={}", edgeId);

        boolean edgeIdExists = edgeRepository.existsById(edgeId); // 存在 -> TRUE

        CheckEdgeIdExistsResponse reply = CheckEdgeIdExistsResponse.newBuilder()
                .setExists(edgeIdExists).build();

        responseStreamObserver.onNext(reply);
        log.info("gRPC EdgeService.CheckEdgeIdExists edge_id={} exists={}", edgeId, edgeIdExists);
        responseStreamObserver.onCompleted();
    }

    @Override
    public void verifyEdgeCredentials(
            VerifyEdgeCredentialsRequest request,
            StreamObserver<VerifyEdgeCredentialsResponse> responseObserver) {
        log.info("gRPC EdgeService.VerifyEdgeCredentials edge_id={}", request.getEdgeId());
        boolean valid = edgeVerify.verifyCredentials(request.getEdgeId(), request.getPassword());

        VerifyEdgeCredentialsResponse reply = VerifyEdgeCredentialsResponse.newBuilder()
                .setValid(valid)
                .build();

        responseObserver.onNext(reply);
        log.info("gRPC EdgeService.VerifyEdgeCredentials edge_id={} valid={}", request.getEdgeId(), valid);
        responseObserver.onCompleted();
    }

    @Override
    public void updateEdgePassword(
            UpdateEdgePasswordRequest request,
            StreamObserver<UpdateEdgePasswordResponse> responseObserver) {
        try {
            log.info("gRPC EdgeService.UpdateEdgePassword edge_id={}", request.getEdgeId());
            edgeVerify.updatePassword(request.getEdgeId(), request.getEdgePassword(), request.getNewEdgePassword());

            UpdateEdgePasswordResponse reply = UpdateEdgePasswordResponse.newBuilder()
                    .setErrorCode("0")
                    .build();

            responseObserver.onNext(reply);
            log.info("gRPC EdgeService.UpdateEdgePassword success edge_id={}", request.getEdgeId());
            responseObserver.onCompleted();
        } catch (ResponseStatusException ex) {
            log.warn("gRPC EdgeService.UpdateEdgePassword failed edge_id={} status={} reason={}",
                    request.getEdgeId(), ex.getStatusCode(), ex.getReason());
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
