package com.redsafetw.ios_service;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.client.discovery.EnableDiscoveryClient;

@SpringBootApplication
@EnableDiscoveryClient
public class IosServiceApplication {

    public static void main(String[] args) {
        SpringApplication.run(IosServiceApplication.class, args);
    }

}
