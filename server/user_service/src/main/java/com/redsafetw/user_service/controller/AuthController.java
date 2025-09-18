package com.redsafetw.user_service.controller;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.service.AuthService;
import com.redsafetw.user_service.service.UserService;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.bind.annotation.RequestMapping;

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

    @PostMapping("/refresh")
    public RefreshResponse refresh(@Valid @RequestBody RefreshRequest refreshRequest) {
        return auth.refresh(refreshRequest);
    }
}