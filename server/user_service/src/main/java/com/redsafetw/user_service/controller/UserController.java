package com.redsafetw.user_service.controller;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.service.AuthService;
import com.redsafetw.user_service.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.bind.annotation.RequestMapping;

/**
 * 使用者服務控制器
 *
 * @create 2025-09-11 11:22 pm
 **/
@RestController
@RequestMapping("/auth")
public class UserController {
    private final UserService user;
    private final AuthService auth;

    @Autowired
    public UserController(UserService user, AuthService auth) {
        this.user = user;
        this.auth = auth;
    }

    @PostMapping("/signup")
    public SignupResponse signup(@RequestBody SignupRequest signupRequest) {
        return user.signup(signupRequest);
    }

    @PostMapping("/signin")
    public SigninResponse signin(@RequestBody SigninRequest signinRequest) {
        return user.signin(signinRequest);
    }

    @PostMapping("/refresh")
    public RefreshResponse refresh(@RequestBody RefreshRequest refreshRequest) {
        return auth.refresh(refreshRequest);
    }
}