package com.redsafetw.notify_service.service;

import com.redsafetw.notify_service.domain.EmailLogDomain;
import com.redsafetw.notify_service.repository.EmailLogRepository;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.OffsetDateTime;

@Service
@RequiredArgsConstructor
@Slf4j
public class EmailLogService {

    private static final int MAX_ERROR_LENGTH = 2000;

    private final EmailLogRepository emailLogRepository;

    @Transactional
    public void log(String from, String to, String subject, String body, boolean success, String errorMessage) {
        EmailLogDomain logEntry = EmailLogDomain.builder()
                .fromAddress(from)
                .toAddress(to)
                .subject(subject)
                .body(body)
                .status(success)
                .errorMessage(trimErrorMessage(errorMessage))
                .sendAt(OffsetDateTime.now())
                .build();

        emailLogRepository.save(logEntry);
        if (!success) {
            log.warn("Mail delivery failed to={} subject={}", to, subject);
        }
    }

    private String trimErrorMessage(String errorMessage) {
        if (errorMessage == null) {
            return null;
        }
        if (errorMessage.length() <= MAX_ERROR_LENGTH) {
            return errorMessage;
        }
        return errorMessage.substring(0, MAX_ERROR_LENGTH);
    }
}
