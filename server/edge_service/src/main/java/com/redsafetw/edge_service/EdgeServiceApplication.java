package com.redsafetw.edge_service;

import com.redsafetw.edge_service.config.EdgeMqttProperties;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

@SpringBootApplication
@EnableConfigurationProperties(EdgeMqttProperties.class)
public class EdgeServiceApplication {
    public static void main(String[] args) {
        SpringApplication.run(EdgeServiceApplication.class, args);
    }
}
