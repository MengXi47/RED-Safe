package com.redsafetw.notify_service;

import com.redsafetw.notify_service.service.MailService;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;

@SpringBootApplication
public class NotifyServiceApplication {
  public static void main(String[] args) {
    SpringApplication.run(NotifyServiceApplication.class, args);
  }
}
