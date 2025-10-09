package com.redsafetw.user_service.controller;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.service.AuthService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

/**
 * 認證控制器
 *
 * @create 2025-09-11 11:22 pm
 **/
@Validated
@RestController
@RequiredArgsConstructor
@RequestMapping("/auth")
public class AuthController {
    private final AuthService auth;

    @PostMapping("/signup")
    public SignupResponse signup(@Valid @RequestBody SignupRequest signupRequest) {
        return auth.signup(signupRequest);
    }

    @PostMapping("/signin")
    public SigninResponse signin(@Valid @RequestBody SigninRequest signinRequest) {
        return auth.signin(signinRequest);
    }

    @PostMapping("/signin/otp")
    public SigninResponse signinWithOtp(@Valid @RequestBody SigninRequest signinRequest) {
        return auth.signinWithOtp(signinRequest);
    }

    @PostMapping("/refresh")
    public RefreshResponse refresh(@Valid @RequestBody RefreshRequest refreshRequest) {
        return auth.refresh(refreshRequest);
    }

    @PostMapping("/create/otp")
    public CreateOTPResponse createOTPR(@NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return auth.createOtp(token);
    }

    @PostMapping("/delete/otp")
    public ErrorCodeResponse deleteOTP(@NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return auth.deleteOtp(token);
    }
}
