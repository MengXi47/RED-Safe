package com.redsafetw.user_service.controller;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.service.BindService;
import com.redsafetw.user_service.service.UserService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

/**
 * 使用者控制器
 *
 * @create 2025-09-14
 **/
@Validated
@RestController
@RequiredArgsConstructor
@RequestMapping("/user")
public class UserController {

    private final BindService bindService;
    private final UserService userService;
    @PostMapping("/bind/{edge_id}")
    public BindResponse bind(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization,
            @PathVariable @NotBlank(message = "125") String edge_id) {
        String token = authorization.replace("Bearer ", "");
        return bindService.bind(token, edge_id);
    }

    @PostMapping("/unbind/{edge_id}")
    public UnbindResponse unbind(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization,
            @PathVariable @NotBlank(message = "125") String edge_id) {
        String token = authorization.replace("Bearer ", "");
        return bindService.unbind(token, edge_id);
    }

    @GetMapping("/list/edge_id")
    public EdgeIdListResponse  getEdgeIdList(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.getEdgeIdList(token);
    }

    @PostMapping("/update/edge_name")
    public UpdataEdgeNameResponse updataEdgeName(
            @Valid @RequestBody UpdataEdgeNameRequest updataEdgeNameRequest,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.updataEdgeName(updataEdgeNameRequest, token);
    }
}