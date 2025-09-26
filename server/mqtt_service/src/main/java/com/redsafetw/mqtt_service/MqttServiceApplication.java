package com.redsafetw.mqtt_service;

import com.redsafetw.mqtt_service.config.MqttProperties;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

@SpringBootApplication
@EnableConfigurationProperties(MqttProperties.class)
public class MqttServiceApplication {

    public static void main(String[] args) {
        SpringApplication.run(MqttServiceApplication.class, args);
    }

}
