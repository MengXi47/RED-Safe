package com.redsafetw.user_service.grpc;

import com.grpc.user.EdgeUserBind;
import com.grpc.user.ListEdgeUsersRequest;
import com.grpc.user.ListEdgeUsersResponse;
import com.grpc.user.UnbindEdgeUserRequest;
import com.grpc.user.UnbindEdgeUserResponse;
import com.grpc.user.UserServiceGrpc;
import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.repository.UserRepository;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.grpc.server.service.GrpcService;
import org.springframework.transaction.annotation.Transactional;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.UUID;

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
        log.info("gRPC UserService.ListEdgeUsers edge_id={}", edgeId);
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

                userBuilder.setUserId(bind.getUserId().toString());

                if (user.getLast_login_at() != null) {
                    userBuilder.setLastOnline(formatDate(user.getLast_login_at()));
                }

                responseBuilder.addUsers(userBuilder.build());
            }, () -> log.warn("User record missing for user_id={} edge_id={}", bind.getUserId(), edgeId));
        }

        var response = responseBuilder.build();
        log.info("gRPC UserService.ListEdgeUsers edge_id={} response_count={}", edgeId, response.getUsersCount());
        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    @Override
    @Transactional
    public void unbindEdgeUser(
            UnbindEdgeUserRequest request,
            StreamObserver<UnbindEdgeUserResponse> responseObserver) {

        String edgeId = request.getEdgeId();
        String userIdRaw = request.getUserId();
        String emailRaw = request.getEmail();
        log.info("gRPC UserService.UnbindEdgeUser edge_id={} user_id={} email={}", edgeId, userIdRaw, emailRaw);

        if (edgeId == null || edgeId.isBlank()) {
            responseObserver.onError(Status.INVALID_ARGUMENT.withDescription("edge_id is required").asRuntimeException());
            return;
        }

        final UUID userId;
        if (userIdRaw != null && !userIdRaw.isBlank()) {
            try {
                userId = UUID.fromString(userIdRaw);
            } catch (IllegalArgumentException ex) {
                responseObserver.onError(Status.INVALID_ARGUMENT.withDescription("user_id must be UUID")
                        .withCause(ex)
                        .asRuntimeException());
                return;
            }
        } else if (emailRaw != null && !emailRaw.isBlank()) {
            var userOpt = userRepository.findByEmail(emailRaw.trim());
            if (userOpt.isEmpty()) {
                log.warn("gRPC UserService.UnbindEdgeUser user_not_found edge_id={} email={}", edgeId, emailRaw);
                responseObserver.onError(Status.NOT_FOUND.withDescription("user_not_found").asRuntimeException());
                return;
            }
            userId = userOpt.get().getUserId();
        } else {
            responseObserver.onError(Status.INVALID_ARGUMENT.withDescription("user_id_or_email_required").asRuntimeException());
            return;
        }

        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edgeId)) {
            log.warn("gRPC UserService.UnbindEdgeUser bind_not_found edge_id={} user_id={}", edgeId, userId);
            responseObserver.onError(Status.NOT_FOUND.withDescription("bind_not_found").asRuntimeException());
            return;
        }

        userEdgeBindRepository.deleteByUserIdAndEdgeId(userId, edgeId);

        responseObserver.onNext(UnbindEdgeUserResponse.newBuilder().setErrorCode("0").build());
        log.info("gRPC UserService.UnbindEdgeUser success edge_id={} user_id={}", edgeId, userId);
        responseObserver.onCompleted();
    }

    private String formatDate(OffsetDateTime dateTime) {
        return ISO_FORMATTER.format(dateTime);
    }
}
