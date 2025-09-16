package com.redsafetw.ios_service.service;

import com.grpc.jwt.JwtServiceGrpc;
import com.grpc.jwt.jwtchkRequset;
import com.grpc.jwt.jwtchkResponse;
import com.redsafetw.ios_service.domain.IosDeviceDomain;
import com.redsafetw.ios_service.dto.IosRegisterRequest;
import com.redsafetw.ios_service.dto.IosRegisterResponse;
import com.redsafetw.ios_service.repository.IosDeviceRepository;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;
import java.util.Optional;
import java.util.UUID;

/**
 * iOS 裝置註冊服務
 */
@Service
@Transactional
@RequiredArgsConstructor
public class IosRegistrationService {
    private static final Logger log = LoggerFactory.getLogger(IosRegistrationService.class);

    private final IosDeviceRepository iosDeviceRepository;
    private final JwtServiceGrpc.JwtServiceBlockingStub jwtBlockingStub;

    public IosRegisterResponse registerDevice(String authorization, IosRegisterRequest request) {
        UUID userId = authenticate(authorization);

        String sanitizedToken = request.getApnsToken().trim();

        IosDeviceDomain device = resolveTargetDevice(userId, sanitizedToken, request.getIosDeviceId());
        device.setUserId(userId);
        device.setApnsToken(sanitizedToken);

        String deviceName = request.getDeviceName();
        device.setDeviceName(deviceName != null ? deviceName.trim() : null);
        device.setLastSeenAt(OffsetDateTime.now());

        IosDeviceDomain saved = iosDeviceRepository.save(device);
        log.info("iOS device registered: deviceId={}, userId={}", saved.getIosDeviceId(), userId);

        return IosRegisterResponse.builder()
                .iosDeviceId(saved.getIosDeviceId())
                .apnsToken(saved.getApnsToken())
                .deviceName(saved.getDeviceName())
                .build();
    }

    private UUID authenticate(String authorization) {
        String token = extractToken(authorization);

        jwtchkResponse response;
        try {
            response = jwtBlockingStub.check(jwtchkRequset.newBuilder().setJwt(token).build());
        } catch (Exception ex) {
            log.error("Failed to verify JWT via user-service", ex);
            throw new ResponseStatusException(HttpStatus.SERVICE_UNAVAILABLE, "USER_SERVICE_UNAVAILABLE");
        }

        if (!response.getChk()) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "INVALID_TOKEN");
        }

        try {
            return UUID.fromString(response.getUserId());
        } catch (IllegalArgumentException ex) {
            log.warn("Invalid user id returned from JWT service: {}", response.getUserId());
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "INVALID_TOKEN");
        }
    }

    private String extractToken(String authorization) {
        if (authorization == null) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "MISSING_AUTHORIZATION_HEADER");
        }

        String[] parts = authorization.trim().split(" ", 2);
        if (parts.length != 2 || !parts[0].equalsIgnoreCase("Bearer") || parts[1].isBlank()) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "INVALID_AUTHORIZATION_HEADER");
        }

        return parts[1].trim();
    }

    private IosDeviceDomain resolveTargetDevice(UUID userId, String apnsToken, UUID deviceId) {

        if (deviceId != null) {
            IosDeviceDomain existing = iosDeviceRepository.findByIosDeviceIdAndUserId(deviceId, userId)
                    .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "IOS_DEVICE_NOT_FOUND"));

            Optional<IosDeviceDomain> conflict = iosDeviceRepository.findByApnsToken(apnsToken);
            if (conflict.isPresent() && !conflict.get().getIosDeviceId().equals(existing.getIosDeviceId())) {
                throw new ResponseStatusException(HttpStatus.CONFLICT, "APNS_TOKEN_IN_USE");
            }

            return existing;
        }

        Optional<IosDeviceDomain> byToken = iosDeviceRepository.findByApnsToken(apnsToken);
        if (byToken.isPresent()) {
            IosDeviceDomain existing = byToken.get();
            existing.setUserId(userId);
            return existing;
        }

        IosDeviceDomain fresh = new IosDeviceDomain();
        fresh.setUserId(userId);
        return fresh;
    }
}
