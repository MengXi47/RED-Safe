package com.redsafetw.gateway;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.netflix.eureka.server.EnableEurekaServer;

@SpringBootApplication
@EnableEurekaServer
public class ServiceRegistryApplication {

    public static void main(String[] args) {
        SpringApplication app = new SpringApplication(ServiceRegistryApplication.class);
        app.setAdditionalProfiles("registry");
        app.run(args);
    }
}
