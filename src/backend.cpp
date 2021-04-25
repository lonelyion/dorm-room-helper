#include "backend.h"
#include "ac-remote.h"
#include <ArduinoJson.h>
#include <DHTesp.h>
#include <inttypes.h> 

#include "pins.h"

WebBackend::WebBackend() {
  web_instance = new AsyncWebServer(8081);
}

WebBackend::WebBackend(uint16_t p) : port(p) {
  web_instance = new AsyncWebServer(port);
}

void WebBackend::begin() {
  web_instance->on("/led", HTTP_ANY, std::bind(&WebBackend::request_led, this, std::placeholders::_1));
  web_instance->on("/sensor", HTTP_GET, std::bind(&WebBackend::request_sensor, this, std::placeholders::_1));
  web_instance->on("/air-conditioner", HTTP_GET | HTTP_POST, std::bind(&WebBackend::request_aircon, this, std::placeholders::_1));
  web_instance->begin();
}

// route: /led
// accept: GET POST
void WebBackend::request_led(AsyncWebServerRequest *request) {
  extern uint8_t led_status;

  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  std::string *response = new std::string();
  doc["result"] = "ok";
  int code = 200;
  if (request->method() == HTTP_GET) {
    // GET led status
    doc["status"] = led_status ? "on" : "off";
  } else if (request->method() == HTTP_POST && request->hasParam("switch", true)) {
    // POST(Set) led status
    AsyncWebParameter *p = request->getParam("switch", true);

    if (p->value() == "on") {
      led_status = HIGH;
      doc["status"] = "on";
    } else if (p->value() == "off") {
      led_status = LOW;
      doc["status"] = "off";
    }

    if (doc["status"].isNull()) {
      doc["result"] = "error: parameter value";
      doc["status"] = led_status ? "on" : "off";
      code = 400;  //Bad Request
    } else {
      pinMode(TEST_LED_PIN, OUTPUT);
      digitalWrite(TEST_LED_PIN, led_status);
    }
  } else {
    doc["result"] = "error: wrong http method";
    code = 405;  //Method Not Allowed
  }
  serializeJson(doc, *response);
  request->send(code, "application/json", response->c_str());
}

// route: /sensor
// accept: GET
void WebBackend::request_sensor(AsyncWebServerRequest *request) {
  extern DHTesp sensor;

  StaticJsonDocument<JSON_OBJECT_SIZE(4)> doc;
  std::string *response = new std::string();
  doc["result"] = "ok";
  int code = 200;

  auto sensor_data = sensor.getTempAndHumidity();
  doc["temperature"] = String(sensor_data.temperature, 2) + "℃";
  doc["humidity"] = String(sensor_data.humidity, 1) + "%";

  serializeJson(doc, *response);
  request->send(code, "application/json", response->c_str());
}

void WebBackend::request_aircon(AsyncWebServerRequest *request) {
  extern ACRemote remote;
  StaticJsonDocument<JSON_OBJECT_SIZE(8)> doc;
  std::string *response = new std::string();
  doc["result"] = "ok";
  int code = 200;
  if (request->method() == HTTP_POST) {
    AsyncWebParameter* p;
    ac_status state_new;
    int tmp;
    if(request->hasParam("power", true)) {
      p = request->getParam("power", true);
      state_new.power = (p->value() == "true") || (p->value() == "True");
    } else goto fail;

    if(request->hasParam("swing", true)) {
      p = request->getParam("swing", true);
      state_new.swing = (p->value() == "true") || (p->value() == "True");
    } else goto fail;

    if(request->hasParam("temperature", true)) {
      p = request->getParam("temperature", true);
      sscanf(p->value().c_str(), "%d", &tmp);
      state_new.temperature = (uint8_t)tmp;
    } else goto fail;
    
    if(request->hasParam("mode", true)) {
      p = request->getParam("mode", true);
      sscanf(p->value().c_str(), "%d", &tmp);
      state_new.mode = (uint8_t)tmp;
    } else goto fail;

    if(request->hasParam("wind_speed", true)) {
      p = request->getParam("wind_speed", true);
      sscanf(p->value().c_str(), "%u", &tmp);
      state_new.wind_speed = (uint8_t)tmp;
    } else goto fail;

    remote.set_value(state_new);
    remote.send_flag = true;
    goto end;
    fail:
      code = 400;
      doc["result"] = "error: missing parameter(s)";
  }

  end:
  doc["power"] = remote.status.power;
  doc["swing"] = remote.status.swing;
  doc["temperature"] = remote.status.temperature;
  doc["mode"] = remote.status.mode;
  doc["wind_speed"] = remote.status.wind_speed;

  serializeJson(doc, *response);
  request->send(code, "application/json", response->c_str());
}