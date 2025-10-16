package com.redsafetw.edge_service.grpc;

import com.grpc.user.ListEdgeUsersRequest;
import com.grpc.user.ListEdgeUsersResponse;
import com.grpc.user.UnbindEdgeUserRequest;
import com.grpc.user.UnbindEdgeUserResponse;
import com.grpc.user.UserServiceGrpc;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

/**
 * user_service gRPC 客戶端
 */
@Component
@RequiredArgsConstructor
public class UserGrpcClient {

    private final UserServiceGrpc.UserServiceBlockingStub stub;

    public ListEdgeUsersResponse listEdgeUsers(String edgeId) {
        ListEdgeUsersRequest request = ListEdgeUsersRequest.newBuilder()
                .setEdgeId(edgeId)
                .build();
        return stub.listEdgeUsers(request);
    }

    public UnbindEdgeUserResponse unbindEdgeUser(String edgeId, String userId, String email) {
        UnbindEdgeUserRequest.Builder builder = UnbindEdgeUserRequest.newBuilder()
                .setEdgeId(edgeId);
        if (userId != null && !userId.isBlank()) {
            builder.setUserId(userId);
        }
        if (email != null && !email.isBlank()) {
            builder.setEmail(email);
        }
        UnbindEdgeUserRequest request = builder.build();
        return stub.unbindEdgeUser(request);
    }
}
