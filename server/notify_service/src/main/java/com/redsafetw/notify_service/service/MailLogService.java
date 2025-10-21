package com.redsafetw.notify_service.service;

import com.redsafetw.notify_service.domain.MailLogDomain;
import com.redsafetw.notify_service.repository.MailLogRepository;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.OffsetDateTime;

@Service
@RequiredArgsConstructor
@Slf4j
public class MailLogService {

    private static final int MAX_ERROR_LENGTH = 2000;

    private final MailLogRepository mailLogRepository;

    @Transactional
    public void log(String from, String to, String subject, String body, boolean success, String errorMessage) {
        MailLogDomain logEntry = MailLogDomain.builder()
                .fromAddress(from)
                .toAddress(to)
                .subject(subject)
                .body(body)
                .status(success)
                .errorMessage(trimErrorMessage(errorMessage))
                .sendAt(OffsetDateTime.now())
                .build();

        mailLogRepository.save(logEntry);
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
