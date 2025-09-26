package com.redsafetw.mqtt_service.config;

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
    /** 符合 Paho URI 格式，例如 tcp:// 或 wss:// */
    private String uri;
    /** MQTT 使用者名稱 */
    private String username;
    /** MQTT 密碼 */
    private String password;
    /** 訊息 QoS，預設 1 */
    private int qos = 1;
}
