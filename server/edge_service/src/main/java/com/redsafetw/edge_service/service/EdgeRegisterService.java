package com.redsafetw.edge_service.service;

import com.redsafetw.edge_service.dto.EdgeRegisterRequest;
import com.redsafetw.edge_service.repository.EdgeRepository;
import com.redsafetw.edge_service.dto.EdgeRegisterResponse;
import com.redsafetw.edge_service.domain.EdgeDeviceDomain;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;

/**
 * 邊緣裝置註冊服務
 *
 * @create 2025-09-14
 */
@Service
@Transactional
@RequiredArgsConstructor
public class EdgeRegisterService {
    private final EdgeRepository edgeRepository;
    private static final Logger log = LoggerFactory.getLogger(EdgeRegisterService.class);

    public EdgeRegisterResponse registerEdge(EdgeRegisterRequest req) {
        if (req.getEdgeId() == null || req.getVersion() == null) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Edge Id and Version Required");
        }

        if (!req.getEdgeId().matches("^RED-[0-9A-F]{8}$")) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Invalid Edge Id format");
        }

        if (edgeRepository.existsByEdgeId(req.getEdgeId())) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "Edge Id already registered");
        }

        log.info("Edge Device Registered Successfully: id: {}", req.getEdgeId());
        EdgeDeviceDomain device = new EdgeDeviceDomain();
        device.setEdgeId(req.getEdgeId());
        device.setVersion(req.getVersion());
        device.setRegisteredAt(OffsetDateTime.now());
        edgeRepository.save(device);

        return EdgeRegisterResponse.builder()
                .edgeId(req.getEdgeId())
                .build();
    }
}