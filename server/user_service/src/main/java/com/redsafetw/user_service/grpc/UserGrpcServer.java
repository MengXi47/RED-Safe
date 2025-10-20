package com.redsafetw.user_service.grpc;

import com.grpc.user.CreateUserProfileRequest;
import com.grpc.user.CreateUserProfileResponse;
import com.grpc.user.EdgeUserBind;
import com.grpc.user.GetUserProfileRequest;
import com.grpc.user.GetUserProfileResponse;
import com.grpc.user.ListEdgeUsersRequest;
import com.grpc.user.ListEdgeUsersResponse;
import com.grpc.user.UnbindEdgeUserRequest;
import com.grpc.user.UnbindEdgeUserResponse;
import com.grpc.user.UserServiceGrpc;
import com.redsafetw.user_service.domain.UserEdgeBindDomain;
import com.redsafetw.user_service.domain.UserDomain;
import com.redsafetw.user_service.domain.UserNotificationSettings;
import com.redsafetw.user_service.grpc.AuthGrpcClient;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import com.redsafetw.user_service.repository.UserNotificationSettingsRepository;
import com.redsafetw.user_service.repository.UserRepository;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.grpc.server.service.GrpcService;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.Locale;
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
    private final UserNotificationSettingsRepository notificationSettingsRepository;
    private final AuthGrpcClient authGrpcClient;

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
                        .setUserName(user.getUserName() == null ? "" : user.getUserName());

                if (bind.getBindAt() != null) {
                    userBuilder.setBindAt(formatDate(bind.getBindAt()));
                }

                userBuilder.setUserId(bind.getUserId().toString());

                try {
                    var profile = authGrpcClient.getSecurityProfile(user.getUserId());
                    String lastLogin = profile.getLastLoginAt();
                    if (!lastLogin.isBlank()) {
                        userBuilder.setLastOnline(lastLogin);
                    }
                } catch (ResponseStatusException ex) {
                    log.warn("listEdgeUsers security profile fetch failed user_id={}", user.getUserId(), ex);
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

    @Override
    @Transactional
    public void createUserProfile(CreateUserProfileRequest request, StreamObserver<CreateUserProfileResponse> responseObserver) {
        UUID userId;
        try {
            userId = UUID.fromString(request.getUserId());
        } catch (IllegalArgumentException ex) {
            responseObserver.onNext(CreateUserProfileResponse.newBuilder().setErrorCode("142").build());
            responseObserver.onCompleted();
            return;
        }

        String email = normalizeEmail(request.getEmail());
        if (email == null) {
            responseObserver.onNext(CreateUserProfileResponse.newBuilder().setErrorCode("124").build());
            responseObserver.onCompleted();
            return;
        }

        if (userRepository.findByUserId(userId).isPresent() || userRepository.existsByEmail(email)) {
            responseObserver.onNext(CreateUserProfileResponse.newBuilder().setErrorCode("133").build());
            responseObserver.onCompleted();
            return;
        }

        UserDomain user = new UserDomain();
        user.setUserId(userId);
        user.setEmail(email);
        user.setUserName(request.getUserName().isBlank() ? null : request.getUserName());
        user.setAvatarUrl(request.getAvatarUrl().isBlank() ? null : request.getAvatarUrl());
        if (!request.getLocale().isBlank()) {
            user.setLocale(request.getLocale());
        }
        if (!request.getTimezone().isBlank()) {
            user.setTimezone(request.getTimezone());
        }
        userRepository.save(user);

        if (!notificationSettingsRepository.existsById(userId)) {
            UserNotificationSettings settings = new UserNotificationSettings();
            settings.setUserId(userId);
            notificationSettingsRepository.save(settings);
        }

        responseObserver.onNext(CreateUserProfileResponse.newBuilder().setErrorCode("0").build());
        responseObserver.onCompleted();
    }

    @Override
    public void getUserProfile(GetUserProfileRequest request, StreamObserver<GetUserProfileResponse> responseObserver) {
        UserDomain user = resolveUser(request);
        if (user == null) {
            responseObserver.onNext(GetUserProfileResponse.newBuilder().build());
            responseObserver.onCompleted();
            return;
        }

        GetUserProfileResponse response = GetUserProfileResponse.newBuilder()
                .setUserId(user.getUserId().toString())
                .setEmail(user.getEmail())
                .setUserName(user.getUserName() == null ? "" : user.getUserName())
                .setAvatarUrl(user.getAvatarUrl() == null ? "" : user.getAvatarUrl())
                .setLocale(user.getLocale() == null ? "" : user.getLocale())
                .setTimezone(user.getTimezone() == null ? "" : user.getTimezone())
                .setCreatedAt(user.getCreatedAt() == null ? "" : formatDate(user.getCreatedAt()))
                .setUpdatedAt(user.getUpdatedAt() == null ? "" : formatDate(user.getUpdatedAt()))
                .build();

        responseObserver.onNext(response);
        responseObserver.onCompleted();
    }

    private UserDomain resolveUser(GetUserProfileRequest request) {
        if (!request.getUserId().isBlank()) {
            try {
                UUID userId = UUID.fromString(request.getUserId());
                return userRepository.findByUserId(userId).orElse(null);
            } catch (IllegalArgumentException ex) {
                return null;
            }
        }
        if (!request.getEmail().isBlank()) {
            String email = normalizeEmail(request.getEmail());
            if (email == null) {
                return null;
            }
            return userRepository.findByEmail(email).orElse(null);
        }
        return null;
    }

    private String formatDate(OffsetDateTime dateTime) {
        return ISO_FORMATTER.format(dateTime);
    }

    private String normalizeEmail(String email) {
        if (email == null) {
            return null;
        }
        String trimmed = email.trim().toLowerCase(Locale.ROOT);
        return trimmed.isEmpty() ? null : trimmed;
    }
}
