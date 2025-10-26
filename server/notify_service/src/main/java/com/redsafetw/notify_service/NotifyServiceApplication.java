package com.redsafetw.notify_service;

import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import com.redsafetw.notify_service.service.EmailService;

@SpringBootApplication
public class NotifyServiceApplication {
    public static void main(String[] args) {
        SpringApplication.run(NotifyServiceApplication.class, args);
    }

    @Bean
    CommandLineRunner sendTestMail(EmailService mailService) {
        return args -> {
            mailService.sendFallAlert(
                    "3b017045@gm.student.ncut.edu.tw",
                    "FUCK",
                    "GAY.GAY.GAY.GAY",
                    "炮房",
                    "2025-11-11 14:32",
                    "臺中市太平區中山路二段 57 號"
            );
        };
    }
}
