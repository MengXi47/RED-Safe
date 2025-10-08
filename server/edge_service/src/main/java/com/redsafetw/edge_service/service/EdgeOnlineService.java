package com.redsafetw.edge_service.service;

import com.redsafetw.edge_service.dto.EdgeOnlineRequest;
import com.redsafetw.edge_service.dto.ErrorCodeResponse;
import com.redsafetw.edge_service.repository.EdgeRepository;
import com.redsafetw.edge_service.grpc.HeartbeatGrpcClient;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;

/**
 * 邊緣裝置上線處理器
 *
 * @create 2025-10-01 by brian
 */
@Service
@Transactional
@RequiredArgsConstructor
public class EdgeOnlineService {
    private static final Logger log = LoggerFactory.getLogger(EdgeOnlineService.class);

    private final EdgeRepository edgeRepository;
    private final HeartbeatGrpcClient heartbeatGrpcClient;

    public ErrorCodeResponse handleOnline(EdgeOnlineRequest request) {
        var device = edgeRepository.findByEdgeId(request.getEdgeId())
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "123"));

        device.setLastOnlineAt(OffsetDateTime.now());
        edgeRepository.save(device);

        try {
            heartbeatGrpcClient.registerEdgeHeartbeat(request.getEdgeId());
        } catch (Exception ex) {
            log.warn("Failed to register heartbeat subscription for edge {}", request.getEdgeId(), ex);
        }

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }
}
