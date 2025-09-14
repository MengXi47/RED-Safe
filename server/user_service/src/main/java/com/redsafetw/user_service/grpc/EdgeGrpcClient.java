package com.redsafetw.user_service.grpc;

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
    public boolean CheckEdgeIdExists(String edge_id) {
        CheckEdgeIdExistsRequest req = CheckEdgeIdExistsRequest.newBuilder()
                .setEdgeId(edge_id)
                .build();
        CheckEdgeIdExistsResponse resp = stub.checkEdgeIdExists(req);
        return resp.getExists();
    }

    /**
     * 通過 gRPC 更新 edge_name
     * @param edge_name new name
     * @param edge_id id
     * @return error_code
     */
    public String UpdataEdgeName(String edge_id, String edge_name) {
        UpdataEdgeNameRequest req = UpdataEdgeNameRequest.newBuilder()
                .setEdgeId(edge_id)
                .setEdgeName(edge_name)
                .build();
        UpdataEdgeNameResponse resp = stub.updataEdgeName(req);
        return resp.getErrorCode();
    }
}