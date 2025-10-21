package com.redsafetw.auth_service.service;

import com.grpc.user.GetUserProfileResponse;
import com.redsafetw.auth_service.grpc.UserGrpcClient;
import com.redsafetw.auth_service.dto.CreateOTPResponse;
import com.redsafetw.auth_service.dto.ErrorCodeResponse;
import com.redsafetw.auth_service.dto.RefreshRequest;
import com.redsafetw.auth_service.dto.RefreshResponse;
import com.redsafetw.auth_service.dto.SigninRequest;
import com.redsafetw.auth_service.dto.SigninResponse;
import com.redsafetw.auth_service.dto.SignupRequest;
import com.redsafetw.auth_service.dto.SignupResponse;
import com.redsafetw.auth_service.service.PasswordManagementService.PasswordChangeResult;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

import java.util.Optional;
import java.util.UUID;

@Service
@Transactional
@RequiredArgsConstructor
public class AuthService {

    private final AccountRegistrationService registrationService;
    private final AccountAuthenticationService authenticationService;
    private final OtpManagementService otpManagementService;
    private final PasswordManagementService passwordManagementService;
    private final SecurityProfileService securityProfileService;
    private final AccessTokenManager accessTokenManager;
    private final UserGrpcClient userGrpcClient;
    private final MailVerificationService mailVerificationService;

    public SignupResponse signup(SignupRequest request) {
        return registrationService.signup(request);
    }

    public SigninResponse signin(SigninRequest request) {
        return authenticationService.signin(request);
    }

    public SigninResponse signinWithOtp(SigninRequest request) {
        return authenticationService.signinWithOtp(request);
    }

    public RefreshResponse refresh(RefreshRequest request) {
        return authenticationService.refresh(request);
    }

    public CreateOTPResponse createOtp(String accessToken) {
        UUID userId = accessTokenManager.requireValidUserId(accessToken);
        return otpManagementService.enableOtp(userId);
    }

    public ErrorCodeResponse deleteOtp(String accessToken) {
        UUID userId = accessTokenManager.requireValidUserId(accessToken);
        return otpManagementService.disableOtp(userId);
    }

    public ErrorCodeResponse sendMailVerification(String accessToken) {
        UUID userId = accessTokenManager.requireValidUserId(accessToken);
        return mailVerificationService.sendMailVerification(userId);
    }

    public ErrorCodeResponse verifyMailCode(String accessToken, String code) {
        UUID userId = accessTokenManager.requireValidUserId(accessToken);
        return mailVerificationService.verifyMailCode(userId, code);
    }

    public PasswordChangeResult changePassword(UUID userId, String currentPassword, String newPassword) {
        return passwordManagementService.changePassword(userId, currentPassword, newPassword);
    }

    public Optional<SecurityProfileService.SecurityProfile> getUserSecurityProfile(UUID userId) {
        return securityProfileService.getSecurityProfile(userId);
    }

    public Optional<GetUserProfileResponse> getUserProfile(UUID userId) {
        return userGrpcClient.getUserProfileByUserId(userId);
    }

    public UUID verifyAccessToken(String token) {
        return accessTokenManager.verifyAccessToken(token);
    }

    public boolean verifyPassword(UUID userId, String rawPassword) {
        return passwordManagementService.verifyPassword(userId, rawPassword);
    }
}
