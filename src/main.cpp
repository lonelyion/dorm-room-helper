#include <Arduino.h>
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
  WiFi.begin(_WIFI_SSID, _WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected.");
  configTime(28800, 0, "192.168.1.1");   //GMT+8, 无夏令时, 路由器的NTP服务端
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  Serial.println("Synchornized time info from OpenWRT NTP.");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);    //用完就关了
  Serial.println("Disconnected WLAN to save power.");
  
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
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!check_time(get_time_string(), sleep_time, wake_time) && nfc->read_and_check_match()) {
    servo->open_the_door();
  }
  delay(1200);
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