package com.redsafetw.bind_service.controller;

import com.redsafetw.bind_service.dto.BindRequest;
import com.redsafetw.bind_service.dto.ErrorCodeResponse;
import com.redsafetw.bind_service.dto.UnBindRequest;
import com.redsafetw.bind_service.service.BindService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

@Validated
@RestController
@RequiredArgsConstructor
@RequestMapping("/bind")
public class BindController {
    private final BindService bindService;
    @PostMapping("/bind")
    public ErrorCodeResponse bind(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization,
            @Valid @RequestBody BindRequest req) {
        String token = authorization.replace("Bearer ", "");
        return bindService.BindUserEdge(token, req);
    }

    @PostMapping("/unbind")
    public ErrorCodeResponse unbind(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization,
            @Valid @RequestBody UnBindRequest req) {
        String token = authorization.replace("Bearer ", "");
        return bindService.UnBindUserEdge(token, req);
    }
}