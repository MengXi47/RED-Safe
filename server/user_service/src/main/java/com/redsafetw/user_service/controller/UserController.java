package com.redsafetw.user_service.controller;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.service.BindService;
import com.redsafetw.user_service.service.EdgeCommandService;
import com.redsafetw.user_service.service.EdgeCommandSseService;
import com.redsafetw.user_service.service.EdgeListService;
import com.redsafetw.user_service.service.UserService;
import jakarta.validation.Valid;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import lombok.RequiredArgsConstructor;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;
import org.springframework.http.MediaType;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

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
    private final EdgeCommandService edgeCommandService;
    private final EdgeListService edgeListService;
    private final EdgeCommandSseService edgeCommandSseService;

    @GetMapping("/info")
    public UserInfoResponse getUserInfo(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.getUserInfo(token);
    }

    @PostMapping("/bind")
    public BindResponse bind(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization,
            @Valid @RequestBody BindRequest req) {
        String token = authorization.replace("Bearer ", "");
        return bindService.bind(token, req);
    }

    @PostMapping("/unbind/{edge_id}")
    public UnbindResponse unbind(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization,
            @PathVariable @NotBlank(message = "125") @Pattern(regexp = "^RED-[0-9A-F]{8}$", message = "120") String edge_id) {
        String token = authorization.replace("Bearer ", "");
        return bindService.unbind(token, edge_id);
    }

    @GetMapping("/list/edge_id")
    public EdgeIdListResponse  getEdgeIdList(
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return edgeListService.getEdgeIdList(token);
    }

    @PostMapping("/update/edge_name")
    public ErrorCodeResponse updataEdgeName(
            @Valid @RequestBody UpdateEdgeNameRequest updateEdgeNameRequest,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.updataEdgeName(updateEdgeNameRequest, token);
    }

    @PostMapping("/update/user_name")
    public ErrorCodeResponse updateUserName(
            @Valid @RequestBody UpdateUserNameRequest updateUserNameRequest,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.updateUserName(updateUserNameRequest, token);
    }

    @PostMapping("/update/password")
    public ErrorCodeResponse updatePassword(
            @Valid @RequestBody UpdateUserPasswordRequest  updateUserPasswordRequest,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.updateUserPassword(updateUserPasswordRequest,  token);
    }

    @PostMapping("/update/edge_password")
    public ErrorCodeResponse updateEdgePassword(
            @Valid @RequestBody UpdateEdgePasswordRequest updateEdgePasswordRequest,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return userService.updateEdgePassword(updateEdgePasswordRequest, token);
    }

    @PostMapping("/edge/command")
    public EdgeCommandResponse sendEdgeCommand(
            @Valid @RequestBody EdgeCommandRequest request,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return edgeCommandService.sendCommand(request, token);
    }

    @GetMapping(value = "/sse/get/command/{trace_id}", produces = MediaType.TEXT_EVENT_STREAM_VALUE)
    public SseEmitter getCommandSse(
            @PathVariable("trace_id") @NotBlank(message = "149")
            @Pattern(regexp = "^[0-9a-fA-F\\-]{36}$", message = "149") String traceId,
            @NotBlank(message = "127") @RequestHeader("Authorization") String authorization) {
        String token = authorization.replace("Bearer ", "");
        return edgeCommandSseService.streamCommandResult(traceId, token);
    }
}
