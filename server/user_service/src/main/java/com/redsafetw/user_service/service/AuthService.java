package com.redsafetw.user_service.service;

import com.redsafetw.user_service.domain.AuthDomain;
import com.redsafetw.user_service.domain.UserDomain;
import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.repository.AuthRepository;
import com.redsafetw.user_service.repository.UserRepository;
import com.redsafetw.user_service.util.*;
import com.warrenstrange.googleauth.GoogleAuthenticator;
import com.warrenstrange.googleauth.GoogleAuthenticatorConfig;
import com.warrenstrange.googleauth.GoogleAuthenticatorKey;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.security.SecureRandom;
import java.time.OffsetDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

/**
 * 認證服務
 *
 * @create 2025-09-13
 */
@Service
@Transactional
@RequiredArgsConstructor
public class AuthService {
    private final UserRepository users;
    private final AuthRepository auths;
    private final AuthRepository authRepository;
    private static final Logger logger = LoggerFactory.getLogger(AuthService.class);
    private static final SecureRandom RANDOM = new SecureRandom();
    private final GoogleAuthenticator gAuth = new GoogleAuthenticator(
            new GoogleAuthenticatorConfig.GoogleAuthenticatorConfigBuilder()
                    .setSecretBits(160)
                    .setCodeDigits(6)
                    .setTimeStepSizeInMillis(30_000)
                    .setWindowSize(1)
                    .build()
    );

    public SignupResponse signup(SignupRequest req) {

        // 驗證Email使否重複
        if (users.existsByEmail(req.getEmail())) {
            logger.info("Signup: {\"user_name\":\"{}\", \"email\":\"{}\", \"password\":\"{}\"} Email已存在",
                    req.getUserName(),
                    req.getEmail(),
                    req.getPassword());
            throw new ResponseStatusException(HttpStatus.CONFLICT, "133");
        }

        // 寫入資料庫
        UserDomain user = new UserDomain();
        user.setEmail(req.getEmail());
        user.setUser_name(req.getUserName());
        user.setUser_password_hash(Argon2id.hash(req.getPassword()));
        user = users.save(user);

        logger.info("Signup: {\"user_name\":\"{}\", \"email\":\"{}\", \"password\":\"{}\"} Signup successful user_id: {}",
                user.getUser_name(),
                user.getEmail(),
                req.getPassword(),
                user.getUserId());
        return SignupResponse.builder()
                .userId(user.getUserId())
                .userName(user.getUser_name())
                .build();
    }

    public SigninResponse signin(SigninRequest req) {

        // 驗證帳號與密碼
        if (!verifyPassword(req.getEmail(), req.getPassword())) {
            logger.info("Signin: {\"email\":\"{}\", \"password\":\"{}\"} 帳號密碼錯誤", req.getEmail(), req.getPassword());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        var user = users.findByEmail(req.getEmail().trim())
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128"));

        if (Boolean.TRUE.equals(user.getOtpEnabled())) {
            throw new ResponseStatusException(HttpStatus.OK, "150");
        }

        String accessToken = JwtService.createToken(user.getUserId());
        String refreshToken = RefreshToken.generateRefreshToken();

        user.setLast_login_at(OffsetDateTime.now());
        users.save(user);

        AuthDomain auth = new AuthDomain();
        auth.setUser(user);
        auth.setRefresh_token(refreshToken);
        auth.setCreated_at(OffsetDateTime.now());
        auth.setExpires_at(OffsetDateTime.now().plusDays(30));
        auth.setRevoked(false);
        auths.save(auth);

        logger.info("Signin: {\"email\":\"{}\", \"password\":\"{}\"} Signin successful user_id: {}",
                req.getEmail(),
                req.getPassword(),
                user.getUserId());
        return SigninResponse.builder()
                .userName(user.getUser_name())
                .accessToken(accessToken)
                .refreshToken(refreshToken)
                .build();
    }

    public SigninResponse signinWithOtp(SigninRequest req) {
        if (!verifyPassword(req.getEmail(), req.getPassword())) {
            logger.info("SigninOTP: {\"email\":\"{}\", \"password\":\"{}\"} 帳號密碼錯誤", req.getEmail(), req.getPassword());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        var user = users.findByEmail(req.getEmail().trim())
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128"));

        if (!Boolean.TRUE.equals(user.getOtpEnabled())) {
            logger.info("SigninOTP: {\"email\":\"{}\"} otp 未啟用", req.getEmail());
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "151");
        }

        boolean verified = verifyOtpOrBackup(user, req.getOtpCode());

        if (!verified) {
            logger.info("SigninOTP: {\"email\":\"{}\"} OTP 驗證失敗", req.getEmail());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "152");
        }

        String accessToken = JwtService.createToken(user.getUserId());
        String refreshToken = RefreshToken.generateRefreshToken();

        user.setLast_login_at(OffsetDateTime.now());
        users.save(user);

        AuthDomain auth = new AuthDomain();
        auth.setUser(user);
        auth.setRefresh_token(refreshToken);
        auth.setCreated_at(OffsetDateTime.now());
        auth.setExpires_at(OffsetDateTime.now().plusDays(30));
        auth.setRevoked(false);
        auths.save(auth);

        logger.info("SigninOTP: {\"email\":\"{}\"} Signin successful with OTP user_id: {}", req.getEmail(), user.getUserId());
        return SigninResponse.builder()
                .userName(user.getUser_name())
                .accessToken(accessToken)
                .refreshToken(refreshToken)
                .build();
    }

    public RefreshResponse refresh(RefreshRequest req) {

        OffsetDateTime now = OffsetDateTime.now();
        Optional<AuthDomain> result = authRepository.findActiveByRefreshToken(req.getRefreshToken(), now);

        if (result.isEmpty()) {
            logger.info("Refresh: {\"refresh_token\":\"{}\"} refresh_token 失效",
                    req.getRefreshToken());
            throw new ResponseStatusException(HttpStatus.CONFLICT, "132");
        }

        AuthDomain auth = result.get();
        UUID user_id = auth.getUser().getUserId();
        String access_token = JwtService.createToken(user_id);

        var user = auth.getUser();
        user.setLast_login_at(OffsetDateTime.now());

        logger.info("Refresh: {\"refresh_token\":\"{}\"} Refresh successful user_id: {}",
                req.getRefreshToken(),
                user_id);
        return RefreshResponse.builder()
                .accessToken(access_token)
                .build();
    }

    /**
     * 驗證密碼（以 Email 查詢）
     * @param email 使用者 Email
     * @param rawPassword 原始密碼
     * @return 若密碼正確則回傳 true，否則 false；查無使用者亦回傳 false
     */
    public boolean verifyPassword(String email, String rawPassword) {
        if (email == null || rawPassword == null) return false;
        var userOpt = users.findByEmail(email.trim());
        if (userOpt.isEmpty()) {
            return false;
        }
        var user = userOpt.get();
        return Argon2id.verify(user.getUser_password_hash(), rawPassword);
    }

    /**
     * 驗證密碼（以 user_id 查詢）
     * @param userId 使用者 ID
     * @param rawPassword 原始密碼
     * @return 若密碼正確則回傳 true，否則 false；查無使用者亦回傳 false
     */
    public boolean verifyPassword(UUID userId, String rawPassword) {
        if (userId == null || rawPassword == null) return false;
        var userOpt = users.findByUserId(userId);
        if (userOpt.isEmpty()) {
            return false;
        }
        var user = userOpt.get();
        return Argon2id.verify(user.getUser_password_hash(), rawPassword);
    }

    /**
     * 建立使用者 OTP 秘鑰與備援驗證碼，並啟用二階段驗證。
     * @param accessToken 使用者 access token
     * @return 建立完成後返回的 OTP 秘鑰
     */
    public CreateOTPResponse createOtp(String accessToken) {
        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("createOtp: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        UserDomain user = users.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        if (Boolean.TRUE.equals(user.getOtpEnabled()) && user.getOtpSecret() != null) {
            logger.info("createOtp: {{\"user_id\":\"{}\"}} 已啟用 OTP", userId);
            throw new ResponseStatusException(HttpStatus.CONFLICT, "153");
        }

        GoogleAuthenticatorKey key = gAuth.createCredentials();
        String otpSecret = key.getKey();
        List<String> backupCodes = generateBackupCodes(3);

        user.setOtpSecret(otpSecret);
        user.setOtpEnabled(Boolean.TRUE);
        user.setOtpBackupCodes(backupCodes.toArray(new String[0]));
        users.save(user);

        logger.info("createOtp: {{\"user_id\":\"{}\"}} OTP secret generated", userId);
        return CreateOTPResponse.builder()
                .otpKey(otpSecret)
                .backupCodes(List.copyOf(backupCodes))
                .build();
    }

    /**
     * 刪除使用者的 OTP 資訊並關閉二階段驗證
     * @param accessToken 使用者 access token
     * @return 刪除成功回傳 0
     */
    public ErrorCodeResponse deleteOtp(String accessToken) {
        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            logger.info("deleteOtp: {\"access_token\":\"{}\"} access_token 失效", accessToken);
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        UserDomain user = users.findByUserId(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.UNAUTHORIZED, "142"));

        if (!Boolean.TRUE.equals(user.getOtpEnabled()) || user.getOtpSecret() == null) {
            logger.info("deleteOtp: {{\"user_id\":\"{}\"}} 尚未啟用 OTP", userId);
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "154");
        }

        user.setOtpSecret(null);
        user.setOtpEnabled(Boolean.FALSE);
        user.setOtpBackupCodes(null);
        users.save(user);

        logger.info("deleteOtp: {{\"user_id\":\"{}\"}} OTP 已停用", userId);
        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }

    private List<String> generateBackupCodes(int count) {
        List<String> codes = new ArrayList<>(count);
        for (int i = 0; i < count; i++) {
            codes.add(String.format("%06d", RANDOM.nextInt(1_000_000)));
        }
        return codes;
    }

    private boolean verifyOtpOrBackup(UserDomain user, String rawCode) {
        String normalized = normalizeOtp(rawCode);
        if (normalized == null) {
            return false;
        }
        return verifyTotp(user.getOtpSecret(), normalized)
                || verifyBackupCode(user, normalized);
    }

    private boolean verifyTotp(String secret, String otpCode) {
        if (secret == null) {
            return false;
        }

        String normalized = normalizeOtp(otpCode);
        if (normalized == null) {
            return false;
        }

        try {
            int code = Integer.parseInt(normalized);
            return gAuth.authorize(secret, code);
        } catch (NumberFormatException ex) {
            logger.warn("verifyTotp: 非數字格式 OTP -> {}", otpCode);
            return false;
        }
    }

    private boolean verifyBackupCode(UserDomain user, String backupCode) {
        String normalized = normalizeOtp(backupCode);
        if (normalized == null) {
            return false;
        }

        String[] codes = user.getOtpBackupCodes();
        if (codes == null || codes.length == 0) {
            return false;
        }
        for (int i = 0; i < codes.length; i++) {
            if (normalized.equals(codes[i])) {
                codes[i] = null; // 標記為已使用
                user.setOtpBackupCodes(codes);
                users.save(user);
                return true;
            }
        }
        return false;
    }

    private String normalizeOtp(String code) {
        if (code == null) {
            return null;
        }
        String sanitized = code.replace(" ", "").trim();
        return sanitized.isEmpty() ? null : sanitized;
    }

    // decodeBase32 and manual TOTP generation removed in favor of GoogleAuthenticator utilities.
}
