//servo.cpp
//Code by Yutao Gu
#include <servo.h>

Servo::Servo(uint8_t p) : pin(p) {}

uint16_t Servo::map(uint8_t angle) {
  //0度对应MIN = 500us，180度对应MAX = 2500us
  // y = 1/180 (-MIN + MAX) x + MIN
  return uint16_t(round(((MAX - MIN) * angle) / 180.0 + MIN));
}

void Servo::attach() {
  ledcSetup(channel, FREQ, TIMER_WIDTH);
  ledcAttachPin(pin, channel);
  return;
}

void Servo::write_angle(uint8_t angle) {
  int ticks = (int)((float)map(angle) / ((float)PERIOD_US / (float)TIMER_WIDTH_TICKS));
  ledcWrite(channel, ticks);
  return;
}

void Servo::open_the_door() {
  this->write_angle(150);
  delay(3000);
  this->write_angle(0);
}
