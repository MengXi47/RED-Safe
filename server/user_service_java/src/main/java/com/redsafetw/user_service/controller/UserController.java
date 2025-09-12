package com.redsafetw.user_service.controller;

import com.redsafetw.user_service.dto.SigninRequest;
import com.redsafetw.user_service.dto.SigninResponse;
import com.redsafetw.user_service.dto.SignupRequest;
import com.redsafetw.user_service.dto.SignupResponse;
import com.redsafetw.user_service.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

/**
 * 使用者服務控制器
 *
 * @create 2025-09-11 11:22 pm
 **/
@RestController
public class UserController {
    private final UserService user;

    @Autowired
    public UserController(UserService user) {
        this.user = user;
    }

    @PostMapping("/signup")
    public SignupResponse signup(@RequestBody SignupRequest signupRequest) {
        return user.signup(signupRequest);
    }

    @PostMapping("/signin")
    public SigninResponse signin(@RequestBody SigninRequest signinRequest) {
        return user.signin(signinRequest);
    }
}