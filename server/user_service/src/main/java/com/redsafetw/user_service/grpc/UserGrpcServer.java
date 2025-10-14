package com.redsafetw.user_service.grpc;

import com.grpc.user.EdgeUserBind;
import com.grpc.user.ListEdgeUsersRequest;
import com.grpc.user.ListEdgeUsersResponse;
import com.grpc.user.UserServiceGrpc;
import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.repository.UserRepository;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.grpc.server.service.GrpcService;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;

/**
 * User gRPC 服務
 */
@GrpcService
@Slf4j
@RequiredArgsConstructor
public class UserGrpcServer extends UserServiceGrpc.UserServiceImplBase {

    private static final DateTimeFormatter ISO_FORMATTER = DateTimeFormatter.ISO_OFFSET_DATE_TIME;

    private final UserEdgeBindRepository userEdgeBindRepository;
    private final UserRepository userRepository;

    @Override
    public void listEdgeUsers(
            ListEdgeUsersRequest request,
            StreamObserver<ListEdgeUsersResponse> responseObserver) {

        String edgeId = request.getEdgeId();
        if (edgeId == null || edgeId.isBlank()) {
            responseObserver.onError(Status.INVALID_ARGUMENT.withDescription("edge_id is required").asRuntimeException());
            return;
        }

        List<UserEdgeBindDomain> binds = userEdgeBindRepository.findByEdgeId(edgeId);
        ListEdgeUsersResponse.Builder responseBuilder = ListEdgeUsersResponse.newBuilder();

        for (UserEdgeBindDomain bind : binds) {
            userRepository.findByUserId(bind.getUserId()).ifPresentOrElse(user -> {
                EdgeUserBind.Builder userBuilder = EdgeUserBind.newBuilder()
                        .setEmail(user.getEmail())
                        .setUserName(user.getUser_name() == null ? "" : user.getUser_name());

                if (bind.getBindAt() != null) {
                    userBuilder.setBindAt(formatDate(bind.getBindAt()));
                }

                if (user.getLast_login_at() != null) {
                    userBuilder.setLastOnline(formatDate(user.getLast_login_at()));
                }

                responseBuilder.addUsers(userBuilder.build());
            }, () -> log.warn("User record missing for user_id={} edge_id={}", bind.getUserId(), edgeId));
        }

        responseObserver.onNext(responseBuilder.build());
        responseObserver.onCompleted();
    }

    private String formatDate(OffsetDateTime dateTime) {
        return ISO_FORMATTER.format(dateTime);
    }
}
