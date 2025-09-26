#pragma once

#include <atomic>
#include <memory>

#include <MQTTAsync.h>

#include "config/mqtt_config.hpp"
#include "handler/message_handler.hpp"
#include "reconnect/reconnect_strategy.hpp"

class MqttSubscriber {
 public:
  MqttSubscriber(MqttConfig config,
                 std::shared_ptr<IMessageHandler> handler,
                 std::shared_ptr<IReconnectStrategy> strategy);
  ~MqttSubscriber();

  void start();
  void stop();
  [[nodiscard]] bool running() const noexcept;

 private:
  static void on_connected(void* context, char* cause);
  static void on_connection_lost(void* context, char* cause);
  static int on_message_arrived(void* context,
                                char* topic_name,
                                int topic_len,
                                MQTTAsync_message* message);
  static void on_connect_success(void* context, MQTTAsync_successData*);
  static void on_connect_failure(void* context, MQTTAsync_failureData*);
  static void on_subscribe_success(void* context, MQTTAsync_successData*);
  static void on_subscribe_failure(void* context, MQTTAsync_failureData*);

  void handle_connected(const char* cause);
  void handle_connection_lost(const char* cause);
  int handle_message(const std::string& topic, const std::string& payload);
  void schedule_reconnect();
  void subscribe();

  MqttConfig config_;
  std::shared_ptr<IMessageHandler> handler_;
  std::shared_ptr<IReconnectStrategy> strategy_;

  MQTTAsync client_{nullptr};
  MQTTAsync_connectOptions conn_opts_ = MQTTAsync_connectOptions_initializer;
  MQTTAsync_disconnectOptions disc_opts_ =
      MQTTAsync_disconnectOptions_initializer;
  MQTTAsync_SSLOptions ssl_opts_ = MQTTAsync_SSLOptions_initializer;
  std::atomic_bool running_{false};
  std::atomic_bool attempting_reconnect_{false};
};

