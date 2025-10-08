package com.redsafetw.heartbeat_service;

import com.redsafetw.heartbeat_service.config.MqttProperties;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

@SpringBootApplication
@EnableConfigurationProperties(MqttProperties.class)
public class HeartbeatServiceApplication {
    public static void main(String[] args) {
        SpringApplication.run(HeartbeatServiceApplication.class, args);
    }
}
