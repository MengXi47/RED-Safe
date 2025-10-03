package com.redsafetw.bind_service.grpc;

import com.grpc.edge.*;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class EdgeGrpcClient {

    // 這個 stub 由上面的 @Bean 建立並交給 Spring 管理
    private final EdgeServiceGrpc.EdgeServiceBlockingStub stub;

    /**
     * 通過 gRPC 確認 edge_id 是否存在
     * @param edge_id edge_id
     * @return True -> 存在
     */
    @SuppressWarnings("BooleanMethodIsAlwaysInverted")
    public boolean CheckEdgeIdExists(String edge_id) {
        CheckEdgeIdExistsRequest req = CheckEdgeIdExistsRequest.newBuilder()
                .setEdgeId(edge_id)
                .build();
        CheckEdgeIdExistsResponse resp = stub.checkEdgeIdExists(req);
        return resp.getExists();
    }

    /**
     * 透過 gRPC 驗證 edge_id 與明文密碼
     * @param edgeId edge_id
     * @param rawPassword 明文密碼
     * @return True -> 驗證成功
     */
    public boolean verifyEdgeCredentials(String edgeId, String rawPassword) {
        VerifyEdgeCredentialsRequest req = VerifyEdgeCredentialsRequest.newBuilder()
                .setEdgeId(edgeId)
                .setPassword(rawPassword)
                .build();
        VerifyEdgeCredentialsResponse resp = stub.verifyEdgeCredentials(req);
        return resp.getValid();
    }
}
