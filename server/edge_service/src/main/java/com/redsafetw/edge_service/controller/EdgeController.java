package com.redsafetw.edge_service.controller;

import com.redsafetw.edge_service.dto.*;
import com.redsafetw.edge_service.service.*;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;

/**
 * 邊緣裝置控制器
 *
 * @create 2025-09-14
 */
@RestController
@RequiredArgsConstructor
@RequestMapping("/edge")
public class EdgeController {
    private final EdgeRegisterService EdgeRegister;

    @PostMapping("/reg")
    public EdgeRegisterResponse registerEdge(@Valid @RequestBody EdgeRegisterRequest edgeRegisterRequest) {
        return EdgeRegister.registerEdge(edgeRegisterRequest);
    }
}