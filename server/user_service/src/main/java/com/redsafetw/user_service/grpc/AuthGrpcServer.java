package com.redsafetw.user_service.grpc;

import com.redsafetw.user_service.util.JWT;
import io.grpc.stub.StreamObserver;
import org.springframework.grpc.server.service.GrpcService;
import com.grpc.jwt.*;

import java.util.UUID;

/**
 * gRPC 服務實作類別
 * 負責處理 JWT 檢查請求，並回傳驗證結果與 userId
 *
 * @create 2025-09-13
 */
@GrpcService
public class AuthGrpcServer extends JwtServiceGrpc.JwtServiceImplBase {

    /**
     * gRPC 方法實作：Check
     * 接收一個 JWT，驗證並解析出 userId
     *
     * @param request          包含 JWT 的請求物件
     * @param responseObserver 用來回傳結果的 gRPC 回呼物件
     */
    @Override
    public void check(jwtchkRequset request, StreamObserver<jwtchkResponse> responseObserver) {
        // 從請求中取得 JWT token
        String token = request.getJwt();

        // 驗證 JWT 並取得對應的使用者 UUID
        UUID userId = JWT.verifyAndGetUserId(token);

        // 判斷 userId 是否有效（不是全 0 UUID）
        boolean ok = !userId.equals(new UUID(0L, 0L));

        // 建立回應物件，包含驗證結果與使用者 ID
        jwtchkResponse reply = jwtchkResponse.newBuilder()
                .setChk(ok)
                .setUserId(ok ? userId.toString() : ".")
                .build();

        // 將回應送回 client，並完成串流
        responseObserver.onNext(reply);
        responseObserver.onCompleted();
    }
}