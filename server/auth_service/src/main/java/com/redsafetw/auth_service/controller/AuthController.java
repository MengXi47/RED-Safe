package com.redsafetw.auth_service.controller;

import com.redsafetw.auth_service.dto.*;
import com.redsafetw.auth_service.service.AuthService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@Validated
@RestController
@RequestMapping("/auth")
@RequiredArgsConstructor
public class AuthController {

    private final AuthService authService;

    @PostMapping("/signup")
    public SignupResponse signup(@Valid @RequestBody SignupRequest request) {
        return authService.signup(request);
    }

    @PostMapping("/signin")
    public SigninResponse signin(@Valid @RequestBody SigninRequest request) {
        return authService.signin(request);
    }

    @PostMapping("/signin/otp")
    public SigninResponse signinWithOtp(@Valid @RequestBody SigninRequest request) {
        return authService.signinWithOtp(request);
    }

    @PostMapping("/refresh")
    public RefreshResponse refresh(@Valid @RequestBody RefreshRequest request) {
        return authService.refresh(request);
    }

    @PostMapping("/create/otp")
    public CreateOTPResponse createOtp(@NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return authService.createOtp(token);
    }

    @PostMapping("/delete/otp")
    public ErrorCodeResponse deleteOtp(@NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return authService.deleteOtp(token);
    }

    @PostMapping("/mail/verify/send")
    public ErrorCodeResponse sendEmailVerification(@Valid @RequestBody SendEmailVerificationRequset request) {
        return authService.sendMailVerification(UUID.fromString(request.getUserId()));
    }

    @PostMapping("/mail/verify")
    public ErrorCodeResponse verifyMailCode(@Valid @RequestBody VerifyMailCodeRequest request) {
        return authService.verifyMailCode(UUID.fromString(request.getUserId()), request.getCode());
    }
}
