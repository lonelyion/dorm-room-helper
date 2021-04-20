#include "backend.h"

WebBackend::WebBackend() {
  web_instance = new AsyncWebServer(8081);
}

WebBackend::WebBackend(uint16_t p) : port(p) {
  web_instance = new AsyncWebServer(port);
}

void WebBackend::begin() {
  web_instance->on("/led", HTTP_POST, std::bind(&WebBackend::request_led, this, std::placeholders::_1));
  web_instance->on("/temp", HTTP_GET, std::bind(&WebBackend::request_temp, this, std::placeholders::_1));
  web_instance->on("/air-conditioner", HTTP_ANY, std::bind(&WebBackend::request_temp, this, std::placeholders::_1));
  web_instance->begin();
}