#include <esp_camera.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <base64.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

char clientId[50];
void mqtt_callback(char* topic, byte* payload, unsigned int msgLength);
WiFiClient wifiClient;
PubSubClient mqttClient(mqtt_server, mqtt_port, mqtt_callback, wifiClient);

void setup_wifi() {
  Serial.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi Connected.  IP Address: ");
  Serial.println(WiFi.localIP());
}

boolean mqtt_nonblock_reconnect() {
  if (!mqttClient.connected()) {
    boolean isConn = mqttClient.connect(clientId);
    Serial.printf("MQTT Client [%s] Connect %s!\n", clientId, (isConn ? "Successful" : "Failed"));
  }
  return mqttClient.connected();
}

void MQTT_picture_base64() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed, Reset");
    ESP.restart();
    return;
  }

  String base64Image = base64::encode(fb->buf, fb->len);

  if (!mqttClient.connected()) {
    Serial.printf("MQTT Client [%s] Connection LOST!\n", clientId);
    mqtt_nonblock_reconnect();
  }

  if (mqttClient.connected()) {
    boolean isPublished = mqttClient.publish(MQTT_PUBLISH_Monitor, base64Image.c_str());
    if (isPublished) {
      Serial.println("Publishing Photo to MQTT Successfully!");
    } else {
      Serial.println("Publishing Photo to MQTT Failed!");
    }
  } else {
    Serial.println("No MQTT Connection, Photo NOT Published!");
  }

  esp_camera_fb_return(fb);
}

void secondary_core_task(void* parameter) {
  while (true) {
    delay(1000);
  }
}

void primary_core_task(void* parameter) {
  while (true) {
    mqtt_nonblock_reconnect(); 
    MQTT_picture_base64();
    delay(10000);
  }
}

void setup() {
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.frame_size = FRAMESIZE_QVGA;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  setup_wifi();

  sprintf(clientId, "ESP32CAM_%04X", random(0xffff));
  mqtt_nonblock_reconnect();

  xTaskCreatePinnedToCore(primary_core_task, "Primary Core Task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(secondary_core_task, "Secondary Core Task", 8192, NULL, 1, NULL, 1);
}

void loop() {
	
}
