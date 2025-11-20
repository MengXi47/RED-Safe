package com.redsafetw.ios_service.controller;

import com.redsafetw.ios_service.dto.IosRegisterRequest;
import com.redsafetw.ios_service.dto.IosRegisterResponse;
import com.redsafetw.ios_service.service.IosRegistrationService;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * iOS 控制器
 */
@RestController
@RequiredArgsConstructor
@RequestMapping(value = "/ios", produces = MediaType.APPLICATION_JSON_VALUE)
public class IosController {
    private final IosRegistrationService iosRegistrationService;

    @PostMapping(path = "/reg", consumes = MediaType.APPLICATION_JSON_VALUE)
    public IosRegisterResponse register(@RequestHeader(name = "Authorization", required = false) String authorization,
                                        @Valid @RequestBody IosRegisterRequest request) {
        return iosRegistrationService.registerDevice(authorization, request);
    }
}
