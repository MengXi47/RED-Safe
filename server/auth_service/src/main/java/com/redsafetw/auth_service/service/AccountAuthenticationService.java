package com.redsafetw.auth_service.service;

import com.grpc.user.GetUserProfileResponse;
import com.redsafetw.auth_service.domain.AuthRefreshToken;
import com.redsafetw.auth_service.domain.AuthUser;
import com.redsafetw.auth_service.dto.RefreshRequest;
import com.redsafetw.auth_service.dto.RefreshResponse;
import com.redsafetw.auth_service.dto.SigninRequest;
import com.redsafetw.auth_service.dto.SigninResponse;
import com.redsafetw.auth_service.repository.AuthRefreshTokenRepository;
import com.redsafetw.auth_service.repository.AuthUserRepository;
import com.redsafetw.auth_service.grpc.UserGrpcClient;
import com.redsafetw.auth_service.util.RefreshToken;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Component;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;
import java.util.Optional;
import java.util.UUID;

@Component
@RequiredArgsConstructor
@Transactional
@Slf4j
public class AccountAuthenticationService {

    private final AuthUserRepository authUserRepository;
    private final AuthRefreshTokenRepository authRefreshTokenRepository;
    private final PasswordManagementService passwordManagementService;
    private final OtpManagementService otpManagementService;
    private final AccessTokenManager accessTokenManager;
    private final UserGrpcClient userGrpcClient;

    public SigninResponse signin(SigninRequest request) {
        AuthUser user = authUserRepository.findByEmail(request.getEmail().trim().toLowerCase())
                .orElseThrow(() -> {
                    log.info("Signin failed user not found email={}", request.getEmail());
                    return new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
                });

        if (!passwordManagementService.verifyPassword(user, request.getPassword())) {
            log.info("Signin failed password mismatch email={}", request.getEmail());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        if (Boolean.TRUE.equals(user.getOtpEnabled())) {
            throw new ResponseStatusException(HttpStatus.OK, "150");
        }

        return buildSigninResponse(user);
    }

    public SigninResponse signinWithOtp(SigninRequest request) {
        AuthUser user = authUserRepository.findByEmail(request.getEmail().trim().toLowerCase())
                .orElseThrow(() -> {
                    log.info("Signin OTP failed user not found email={}", request.getEmail());
                    return new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
                });

        if (!passwordManagementService.verifyPassword(user, request.getPassword())) {
            log.info("Signin OTP failed password mismatch email={}", request.getEmail());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "128");
        }

        if (!Boolean.TRUE.equals(user.getOtpEnabled())) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "151");
        }

        boolean verified = otpManagementService.verifyOtpOrBackup(user, request.getOtpCode());
        if (!verified) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "152");
        }

        return buildSigninResponse(user);
    }

    public RefreshResponse refresh(RefreshRequest request) {
        OffsetDateTime now = OffsetDateTime.now();
        AuthRefreshToken token = authRefreshTokenRepository
                .findActiveByRefreshToken(request.getRefreshToken(), now)
                .orElseThrow(() -> {
                    log.info("Refresh token invalid token={}", request.getRefreshToken());
                    return new ResponseStatusException(HttpStatus.CONFLICT, "132");
                });

        AuthUser user = token.getUser();
        UUID userId = user.getUserId();
        String accessToken = accessTokenManager.createAccessToken(userId);

        user.setLastLoginAt(now);
        authUserRepository.save(user);

        return RefreshResponse.builder()
                .accessToken(accessToken)
                .build();
    }

    private SigninResponse buildSigninResponse(AuthUser user) {
        OffsetDateTime now = OffsetDateTime.now();
        String accessToken = accessTokenManager.createAccessToken(user.getUserId());
        String refreshTokenValue = RefreshToken.generate();

        user.setLastLoginAt(now);
        authUserRepository.save(user);

        AuthRefreshToken refreshToken = new AuthRefreshToken();
        refreshToken.setUser(user);
        refreshToken.setRefreshToken(refreshTokenValue);
        refreshToken.setExpiresAt(now.plusDays(30));
        authRefreshTokenRepository.save(refreshToken);

        Optional<GetUserProfileResponse> profile = userGrpcClient.getUserProfileByUserId(user.getUserId());
        String userName = profile.map(GetUserProfileResponse::getUserName).orElse("");

        return SigninResponse.builder()
                .userName(userName)
                .accessToken(accessToken)
                .refreshToken(refreshTokenValue)
                .build();
    }
}
