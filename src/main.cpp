#include <Arduino.h>
#include <Servo.h>
#include <nfc.h>

Servo *servo;
NfcReader *nfc;

int t = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   //串口输出，波特率115200
  
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
    while(1) {  } //失败了就不要往下执行了
  }
  Serial.println("Finished setting up NFC module.");
  nfc->print_version_data();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(nfc->read_and_check_match()) {
    servo->open_the_door();
  }
  delay(1200);
}