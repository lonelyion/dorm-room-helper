#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <string>
#include <time.h>

#include <servo.h>
#include <nfc.h>
#include <secrets.h>

Servo *servo;
NfcReader *nfc;

const std::string sleep_time = "00:30";
const std::string wake_time = "07:00";

inline std::string get_time_string();
inline bool check_time(const std::string&, const std::string&, const std::string&);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   //串口输出，波特率115200
  
  //连接WiFi并进行时间同步
  Serial.printf("Connectting to WLAN: %s ", _WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_WIFI_SSID, _WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print(".");
    //ESP.restart();
  }
  Serial.println(" Connected.");
  configTime(28800, 0, "192.168.1.1");   //GMT+8, 无夏令时, 路由器的NTP服务端
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  Serial.println("Synchornized time info from OpenWRT NTP.");
  
  // 电机
  Serial.println("Setting up Servo...");
  servo = new Servo(26);   //伺服电机实例
  servo->attach();
  servo->write_angle(0);
  Serial.println("Finished setting up Servo.");

  // NFC
  Serial.println("Setting up PN532 NFC module...");
  nfc = new NfcReader(true);    //开启debug输出
  bool nfc_status = nfc->initialize();
  if(!nfc_status) {
    Serial.println("Failed to set up PN532, please check the connection.");
    while(1) {  }   //失败了就不要往下执行了
  }
  Serial.println("Finished setting up NFC module.");
  nfc->print_version_data();

  //OTA module
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
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
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

std::string get_time_string() {
  char buff[sizeof("hh:mm")];
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
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
  if(_ENABLE_SLEEP) {
    while(check_time(get_time_string(), sleep_time, wake_time)) {
      delay(10000);
      return;
    }
  }
  if(nfc->read_and_check_match()) {
    servo->open_the_door();
  }
  delay(500);
}