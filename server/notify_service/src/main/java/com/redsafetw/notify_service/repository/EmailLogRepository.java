package com.redsafetw.notify_service.repository;

import com.redsafetw.notify_service.domain.EmailLogDomain;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface EmailLogRepository extends JpaRepository<EmailLogDomain, Long> {
}
