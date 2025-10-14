package com.redsafetw.edge_service.grpc;

import com.grpc.user.ListEdgeUsersRequest;
import com.grpc.user.ListEdgeUsersResponse;
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
}
