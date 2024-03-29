#ifndef _WEB_H
#define _WEB_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <functional>

class WebBackend {
 public:
  WebBackend();
  WebBackend(uint16_t p);
  
  void request_led(AsyncWebServerRequest *request);
  void request_sensor(AsyncWebServerRequest *request);
  void request_aircon(AsyncWebServerRequest *request);

  void begin();

 private:
  uint16_t port = 8081;
  AsyncWebServer *web_instance;
};

#endif