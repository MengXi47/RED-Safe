package com.redsafetw.auth_service.controller;

import com.redsafetw.auth_service.dto.CreateOTPResponse;
import com.redsafetw.auth_service.dto.ErrorCodeResponse;
import com.redsafetw.auth_service.dto.RefreshRequest;
import com.redsafetw.auth_service.dto.RefreshResponse;
import com.redsafetw.auth_service.dto.SigninRequest;
import com.redsafetw.auth_service.dto.SigninResponse;
import com.redsafetw.auth_service.dto.SignupRequest;
import com.redsafetw.auth_service.dto.SignupResponse;
import com.redsafetw.auth_service.service.AuthService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

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
}
