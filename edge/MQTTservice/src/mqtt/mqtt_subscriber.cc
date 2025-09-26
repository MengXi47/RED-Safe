#include "mqtt/mqtt_subscriber.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

#include "util/environment.hpp"

MqttSubscriber::MqttSubscriber(
    MqttConfig config,
    std::shared_ptr<IMessageHandler> handler,
    std::shared_ptr<IReconnectStrategy> strategy)
    : config_(std::move(config)),
      handler_(std::move(handler)),
      strategy_(std::move(strategy)) {
  if (!handler_ || !strategy_) {
    throw std::invalid_argument("Handler 與 Strategy 不可為空");
  }
}

MqttSubscriber::~MqttSubscriber() {
  stop();
  if (client_ != nullptr) {
    MQTTAsync_destroy(&client_);
    client_ = nullptr;
  }
}

void MqttSubscriber::start() {
  if (running_.exchange(true)) {
    return;
  }

  int rc = MQTTAsync_create(
      &client_,
      config_.server_uri.c_str(),
      config_.client_id.c_str(),
      MQTTCLIENT_PERSISTENCE_NONE,
      nullptr);
  if (rc != MQTTASYNC_SUCCESS) {
    running_ = false;
    throw std::runtime_error(
        "建立 MQTT client 失敗, code=" + std::to_string(rc));
  }

  MQTTAsync_setCallbacks(
      client_,
      this,
      &MqttSubscriber::on_connection_lost,
      &MqttSubscriber::on_message_arrived,
      nullptr);
  MQTTAsync_setConnected(client_, this, &MqttSubscriber::on_connected);

  conn_opts_ = MQTTAsync_connectOptions_initializer;
  conn_opts_.automaticReconnect = 1;
  conn_opts_.minRetryInterval = static_cast<int>(config_.reconnect_min.count());
  conn_opts_.maxRetryInterval = static_cast<int>(config_.reconnect_max.count());
  conn_opts_.keepAliveInterval = 20;
  conn_opts_.cleansession = 0;
  conn_opts_.onSuccess = &MqttSubscriber::on_connect_success;
  conn_opts_.onFailure = &MqttSubscriber::on_connect_failure;
  conn_opts_.context = this;

  if (!config_.username.empty()) {
    conn_opts_.username = config_.username.c_str();
  }
  if (!config_.password.empty()) {
    conn_opts_.password = config_.password.c_str();
  }

  ssl_opts_ = MQTTAsync_SSLOptions_initializer;
  ssl_opts_.sslVersion = MQTT_SSL_VERSION_TLS_1_2;
  const bool trust_all = util::env::get_env_bool("MQTT_SSL_TRUST_ALL", true);
  ssl_opts_.enableServerCertAuth = trust_all ? 0 : 1;
  ssl_opts_.verify = trust_all ? 0 : 1;
  conn_opts_.ssl = &ssl_opts_;

  disc_opts_ = MQTTAsync_disconnectOptions_initializer;
  disc_opts_.timeout = 2000;

  std::cout << "[INFO] Connecting to MQTT broker at " << config_.server_uri
            << '\n';
  rc = MQTTAsync_connect(client_, &conn_opts_);
  if (rc != MQTTASYNC_SUCCESS) {
    std::cerr << "[WARN] 啟動連線失敗 (code=" << rc << ")，將持續重試"
              << std::endl;
    schedule_reconnect();
  }
}

void MqttSubscriber::stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (client_ == nullptr) {
    return;
  }
  if (MQTTAsync_isConnected(client_)) {
    const int rc = MQTTAsync_disconnect(client_, &disc_opts_);
    if (rc != MQTTASYNC_SUCCESS) {
      std::cerr << "[WARN] 中斷連線失敗 (code=" << rc << ")" << std::endl;
    }
  }
}

bool MqttSubscriber::running() const noexcept {
  return running_.load();
}

void MqttSubscriber::on_connected(void* context, char* cause) {
  if (auto* self = static_cast<MqttSubscriber*>(context)) {
    self->handle_connected(cause);
  }
}

void MqttSubscriber::on_connection_lost(void* context, char* cause) {
  if (auto* self = static_cast<MqttSubscriber*>(context)) {
    self->handle_connection_lost(cause);
  }
}

int MqttSubscriber::on_message_arrived(
    void* context,
    char* topic_name,
    int topic_len,
    MQTTAsync_message* message) {
  auto* self = static_cast<MqttSubscriber*>(context);
  if (self == nullptr || message == nullptr) {
    return 0;
  }

  std::string topic;
  if (topic_name != nullptr) {
    topic.assign(
        topic_name,
        topic_len > 0 ? topic_len : static_cast<int>(std::strlen(topic_name)));
  }

  std::string payload;
  if (message->payload != nullptr && message->payloadlen > 0) {
    payload.assign(
        static_cast<const char*>(message->payload), message->payloadlen);
  }

  int processed = self->handle_message(topic, payload);

  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topic_name);
  return processed;
}

void MqttSubscriber::on_connect_success(void* context, MQTTAsync_successData*) {
  if (auto* self = static_cast<MqttSubscriber*>(context)) {
    self->handle_connected("connect onSuccess called");
  }
}

void MqttSubscriber::on_connect_failure(
    void* context, MQTTAsync_failureData* response) {
  auto* self = static_cast<MqttSubscriber*>(context);
  if (self == nullptr) {
    return;
  }
  const int code = response ? response->code : -1;
  std::cerr << "[WARN] MQTT 連線失敗 (code=" << code << ")" << std::endl;
  self->schedule_reconnect();
}

void MqttSubscriber::on_subscribe_success(
    void* context, MQTTAsync_successData*) {
  if (auto* self = static_cast<MqttSubscriber*>(context)) {
    std::cout << "[INFO] 已訂閱 topic '" << self->config_.topic << "'" << '\n';
  }
}

void MqttSubscriber::on_subscribe_failure(
    void* context, MQTTAsync_failureData* response) {
  auto* self = static_cast<MqttSubscriber*>(context);
  if (self == nullptr) {
    return;
  }
  const int code = response ? response->code : -1;
  std::cerr << "[ERROR] 訂閱失敗 (code=" << code << ")" << std::endl;
  self->schedule_reconnect();
}

void MqttSubscriber::handle_connected(const char* cause) {
  std::cout << "[INFO] Connected to MQTT broker";
  if (cause != nullptr) {
    std::cout << " (cause: " << cause << ')';
  }
  std::cout << '\n';
  attempting_reconnect_ = false;
  subscribe();
}

void MqttSubscriber::handle_connection_lost(const char* cause) {
  if (!running_) {
    return;
  }
  std::cerr << "[WARN] MQTT connection lost";
  if (cause != nullptr) {
    std::cerr << ": " << cause;
  }
  std::cerr << '\n';
  schedule_reconnect();
}

int MqttSubscriber::handle_message(
    const std::string& topic, const std::string& payload) {
  handler_->on_message(topic, payload);
  return 1;
}

void MqttSubscriber::subscribe() {
  if (client_ == nullptr) {
    return;
  }

  MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
  opts.context = this;
  opts.onSuccess = &MqttSubscriber::on_subscribe_success;
  opts.onFailure = &MqttSubscriber::on_subscribe_failure;

  const int rc =
      MQTTAsync_subscribe(client_, config_.topic.c_str(), config_.qos, &opts);
  if (rc != MQTTASYNC_SUCCESS) {
    std::cerr << "[ERROR] 發起訂閱失敗 (code=" << rc << ")" << std::endl;
    schedule_reconnect();
  } else {
    std::cout << "[INFO] 正在訂閱 topic '" << config_.topic
              << "' QoS=" << config_.qos << std::endl;
  }
}

void MqttSubscriber::schedule_reconnect() {
  if (!running_) {
    return;
  }
  bool expected = false;
  if (!attempting_reconnect_.compare_exchange_strong(expected, true)) {
    return;
  }

  std::thread([this]() {
    auto delay = strategy_->initial_delay();
    while (running_) {
      std::this_thread::sleep_for(delay);
      if (!running_) {
        break;
      }
      std::cout << "[INFO] 嘗試重新連線..." << std::endl;
      const int rc = MQTTAsync_connect(client_, &conn_opts_);
      if (rc == MQTTASYNC_SUCCESS) {
        std::cout << "[INFO] 已送出重新連線請求" << std::endl;
        break;
      }
      std::cerr << "[WARN] 重新連線請求失敗 (code=" << rc << ")，將在 "
                << delay.count() << " 秒後再試" << std::endl;
      delay = strategy_->next_delay(delay);
    }
    attempting_reconnect_ = false;
  }).detach();
}
