package com.redsafetw.bind_service.grpc;

import com.grpc.auth.AuthServiceGrpc;
import com.grpc.auth.accesstokenchkRequset;
import com.grpc.auth.accesstokenchkResponse;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class AuthGrpcClient {
    private final AuthServiceGrpc.AuthServiceBlockingStub authStub;

    public String chkAccessToken(String accessToken) {
        accesstokenchkResponse res = authStub.checkAccessToken(
                accesstokenchkRequset.newBuilder().setAccesstoken(accessToken).build());

        if (!res.getChk()) {
            return null;
        }

        return res.getUserId();
    }
}
