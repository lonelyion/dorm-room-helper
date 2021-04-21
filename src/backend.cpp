#include "backend.h"

#include <ArduinoJson.h>
#include <DHTesp.h>

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
  web_instance->on("/air-conditioner", HTTP_ANY, std::bind(&WebBackend::request_aircon, this, std::placeholders::_1));
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
    serializeJson(doc, *response);
    request->send(200, "application/json", response->c_str());
    return;
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
