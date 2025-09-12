package com.redsafetw.user_service.service;

import com.redsafetw.user_service.dto.*;
        import com.redsafetw.user_service.domain.Userdomain;
import com.redsafetw.user_service.util.PasswordCrypto;
import com.redsafetw.user_service.repository.UserRepository;
import jakarta.transaction.Transactional;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.nio.charset.StandardCharsets;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import java.util.Base64;
import java.util.UUID;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

/**
 * 使用者服務
 *
 * @create 2025-09-11 11:49 pm
 */
@Service
@Transactional
public class UserService {
    private final UserRepository users;

    // TODO: 將密鑰改為從環境變數/設定檔載入，並定期輪替
    private static final String JWT_HS256_SECRET = "redsafe";
    private static final long ACCESS_TOKEN_MINUTES = 10;
    private static final long REFRESH_TOKEN_DAYS = 30;

    private static String b64Url(byte[] data) {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(data);
    }

    private static String hmacSha256(String data) {
        try {
            Mac mac = Mac.getInstance("HmacSHA256");
            mac.init(new SecretKeySpec(UserService.JWT_HS256_SECRET.getBytes(StandardCharsets.UTF_8), "HmacSHA256"));
            return b64Url(mac.doFinal(data.getBytes(StandardCharsets.UTF_8)));
        } catch (Exception e) {
            throw new IllegalStateException("JWT HMAC 失敗", e);
        }
    }

    private static String issueJwtHs256(String subject, OffsetDateTime issuedAt, OffsetDateTime expiresAt, String type) {
        String headerJson = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
        String payloadJson = String.format("{\"sub\":\"%s\",\"iat\":%d,\"exp\":%d,\"jti\":\"%s\",\"typ\":\"%s\"}",
                subject, issuedAt.toEpochSecond(), expiresAt.toEpochSecond(), UUID.randomUUID(), type);
        String header = b64Url(headerJson.getBytes(StandardCharsets.UTF_8));
        String payload = b64Url(payloadJson.getBytes(StandardCharsets.UTF_8));
        String signingInput = header + "." + payload;
        String signature = hmacSha256(signingInput);
        return signingInput + "." + signature;
    }

    public UserService(UserRepository users) {
        this.users = users;
    }

    public SignupResponse signup(SignupRequest req) {
        if (users.existsByEmail(req.getEmail())) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "Email already registered");
        }
        Userdomain user = new Userdomain();
        user.setEmail(req.getEmail());
        user.setUser_name(req.getUser_name());
        user.setPassword_hash(PasswordCrypto.hash(req.getPassword()));
        user = users.save(user);

        return new SignupResponse(user.getUser_id(), user.getUser_name());
    }

    public SigninResponse signin(SigninRequest req) {
        // 基本檢查
        if (req == null || req.getEmail() == null || req.getPassword() == null) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Email / password 不可為空");
        }

        // 以 Email 取得使用者
        var userOpt = users.findByEmail(req.getEmail().trim());
        if (userOpt.isEmpty()) {
            // 不揭露是帳號還是密碼錯，統一回 401
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "Invalid credentials");
        }

        var user = userOpt.get();

        // 驗證密碼（使用 Spring Security Crypto 的 Argon2id）
        boolean ok = PasswordCrypto.verify(user.getPassword_hash(), req.getPassword());
        if (!ok) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "Invalid credentials");
        }

        // 簽發 JWT 與 Refresh Token（示範使用 HS256，請將密鑰改成外部設定）
        OffsetDateTime now = OffsetDateTime.now(ZoneOffset.UTC);
        OffsetDateTime accessExp = now.plusMinutes(ACCESS_TOKEN_MINUTES);
        OffsetDateTime refreshExp = now.plusDays(REFRESH_TOKEN_DAYS);

        String accessToken = issueJwtHs256(user.getUser_id().toString(), now, accessExp, "access");
        String refreshToken = issueJwtHs256(user.getUser_id().toString(), now, refreshExp, "refresh");

        return SigninResponse.builder()
                .user_id(user.getUser_id())
                .user_name(user.getUser_name())
                .access_token(accessToken)
                .refresh_token(refreshToken)
                .accessTokenExpiresAt(accessExp)
                .build();
    }
}