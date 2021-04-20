//System Headers
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <DHTesp.h>

#include <ctime>
#include <string>

//Custom Headers
#include "backend.h"
#include "ir-remote.h"
#include "nfc.h"
#include "secrets.h"
#include "servo.h"

//Modules
Servo *servo;
NfcReader *nfc;
ir_remote *remote;
WebBackend *backend;
DHTesp dhtSensor1;
TempAndHumidity sensor1Data;

//Pins
const uint8_t SERVO_PIN = 27;
const uint8_t IR_PIN = 13;

const std::string sleep_time = "00:30";
const std::string wake_time = "07:00";

bool led_status = false;

inline std::string get_time_string();
inline bool check_time(const std::string &, const std::string &, const std::string &);

void handleRequest(AsyncWebServerRequest *request) {
  Serial.println("Received request");
  request->send(200, "text/plain", "Hello World!");
}

void setup() {
  Serial.begin(115200);  //串口输出，波特率115200

  //连接WiFi并进行时间同步
  Serial.printf("Connectting to WLAN: %s ", _WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_WIFI_SSID, _WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print(".");
    //ESP.restart();
  }
  Serial.println(" Connected.");
  configTime(28800, 0, "ntp.aliyun.com");  //GMT+8, 无夏令时, 路由器的NTP服务端
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  Serial.println("Synchornized time info from NTP.");

  // 电机
  Serial.println("Setting up Servo...");
  servo = new Servo(SERVO_PIN);  //伺服电机实例
  servo->attach();
  servo->write_angle(0);
  Serial.println("Finished setting up Servo.");

  // NFC
  Serial.println("Setting up PN532 NFC module...");
  nfc = new NfcReader(true, 3);  //开启debug输出
  bool nfc_status = nfc->initialize();
  if (!nfc_status) {
    Serial.println("Failed to set up PN532, please check the connection.");
    //while(1) {  }   //失败了就不要往下执行了
  }
  Serial.println("Finished setting up NFC module.");
  nfc->print_version_data();

  // 红外空调遥控，接收来自串口的指令
  remote = new ir_remote(IR_PIN);
  remote->set_value(true, true, 23, 1, 0);

  //OTA module
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else  // U_SPIFFS
          type = "filesystem";
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();

  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  backend = new WebBackend(80);
  backend->begin();

  pinMode(26, OUTPUT);

  dhtSensor1.setup(14, DHTesp::DHT11);
}

std::string get_time_string() {
  char buff[sizeof("hh:mm")];
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  strftime(buff, sizeof(buff), "%H:%M", &timeinfo);
  return buff;
}

bool check_time(const std::string &now, const std::string &low, const std::string &high) {
  return (now >= low) && (now <= high);
}

void loop() {
  ArduinoOTA.handle();
  // 夜间省电，不响应NFC信号
  if (_ENABLE_SLEEP) {
    while (check_time(get_time_string(), sleep_time, wake_time)) {
      delay(10000);
      //return;
    }
  }
  // 尝试读取NFC ID并与放行名单比对
  if (nfc->read_and_check_match()) {
    servo->open_the_door();
    led_status = !led_status;
    digitalWrite(26, led_status);
  }

  //接收到了空调指令并发送红外信号
  while (Serial.available() > 0) {
    String cmd = Serial.readString();
    int p, s, t, m, ws;
    if (sscanf(cmd.c_str(), "%d %d %d %d %d", &p, &s, &t, &m, &ws) == 5) {
      remote->set_value(p, s, t, m, ws);
      remote->send_signal();
      Serial.printf("Received and sent : %d %d %d %d %d\n", p, s, t, m, ws);
    }
  }

  delay(500);
  sensor1Data = dhtSensor1.getTempAndHumidity();
  Serial.println("Temp: " + String(sensor1Data.temperature,2) + "'C Humidity: " + String(sensor1Data.humidity,1) + "%");
}

/*
void WebBackend::request_led(AsyncWebServerRequest *request) {
  pinMode(2, OUTPUT);
  led_status = !led_status;
  digitalWrite(2, led_status);
  if (led_status) {
    request->send(200, "text/plain", "The LED is ON");
  } else {
    request->send(200, "text/plain", "The LED is OFF");
  }
}
*/