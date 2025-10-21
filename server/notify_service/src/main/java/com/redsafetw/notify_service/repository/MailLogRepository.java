package com.redsafetw.notify_service.repository;

import com.redsafetw.notify_service.domain.MailLogDomain;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface MailLogRepository extends JpaRepository<MailLogDomain, Long> {
}
