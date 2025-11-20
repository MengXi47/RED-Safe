package com.redsafetw.edge_service.service;

import com.redsafetw.edge_service.dto.EdgeRegisterRequest;
import com.redsafetw.edge_service.dto.ErrorCodeResponse;
import com.redsafetw.edge_service.repository.EdgeRepository;
import com.redsafetw.edge_service.domain.EdgeDeviceDomain;
import com.redsafetw.edge_service.util.Argon2id;
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

    public ErrorCodeResponse registerEdge(EdgeRegisterRequest req) {
        if (edgeRepository.existsByEdgeId(req.getEdgeId())) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "146");
        }

        String hashedPassword = Argon2id.hash(req.getEdgePassword());

        log.info("Edge Device Registered Successfully: id: {}", req.getEdgeId());
        EdgeDeviceDomain device = new EdgeDeviceDomain();
        device.setEdgeId(req.getEdgeId());
        device.setEdgePasswordHash(hashedPassword);
        device.setVersion(req.getVersion());
        OffsetDateTime now = OffsetDateTime.now();
        device.setRegisteredAt(now);
        device.setLastOnlineAt(now);
        edgeRepository.save(device);

        return ErrorCodeResponse.builder()
                .errorCode("0")
                .build();
    }
}
