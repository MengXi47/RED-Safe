package com.redsafetw.user_service.config;

import lombok.Getter;
import lombok.Setter;
import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * MQTT 連線設定
 */
@Getter
@Setter
@ConfigurationProperties(prefix = "mqtt")
public class MqttProperties {
    /** MQTT broker URI, e.g. tcp:// or wss:// */
    private String uri;
    /** MQTT username */
    private String username;
    /** MQTT password */
    private String password;
    /** Publish QoS (default 1) */
    private int qos = 1;
}
