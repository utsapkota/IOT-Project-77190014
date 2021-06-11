// Import required libraries

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include "SPIFFS.h"
#include "creds.h"

int pos = 0;
Servo myservo;

// Set LED GPIO
const int frontdoorsledPin = 2;
const int backdoorsledPin = 23;
const char* PARAM_INPUT_1 = "state";

int frontdoorsledState = LOW;
int backdoorsledState = LOW;



const char* PARAM_INPUT_1_TIMER = "state";
const char* PARAM_INPUT_2_TIMER = "value";
String timerSliderValue = "10";
String sliderValue = "0";

const int output_timer = 21;
const int output_timer2 = 19;



AsyncWebServer server(80);
String processor(const String& var) {
  //Serial.println(var);
  if (var == "frontdoors") {
    String buttons = "";
    buttons += "<label class=\"switch ml-auto\"><input type=\"checkbox\" id=\"switch-light-6\" onchange=\"toggleCheckbox(this)\"></label>";
    return buttons;
  }
  if (var == "backdoors") {
    String buttons = "";
    buttons += "<label class=\"switch ml-auto\"><input type=\"checkbox\" id=\"switch-light-7\" onchange=\"toggleCheckbox2(this)\"></label>";
    return buttons;
  }
  if (var == "OPENPLACEHOLDER") {
    String buttons = "";
    buttons += "<button data-action=\"open\" type=\"button\" class=\"btn btn-secondary doors-control\" onclick=\"toggleOpen()\">Open</button>";
    return buttons;
  }
  if (var == "CLOSEPLACEHOLDER") {
    String buttons = "";
    buttons += "<button data-action=\"close\" type=\"button\" class=\"btn btn-secondary doors-control\" onclick=\"toggleClose()\">Close</button>";
    return buttons;
  }
   if (var == "SLIDERVALUE") {
    return sliderValue;
  }
  if (var == "TIMERVALUE") {
    return timerSliderValue;
  }
  if (var == "BUTTONPLACEHOLDER") {
    String buttons = "";
    String outputStateValue = outputState();
    buttons += "<label class=\"switch2\"><input type=\"checkbox\" onchange=\"toggleCheckbox_timer(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState() {
  if (digitalRead(output_timer)) {
    return "checked";
  }
  else {
    return "";
  }
  return "";
}




void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  myservo.attach(13);

  pinMode(frontdoorsledPin, OUTPUT);
  pinMode(backdoorsledPin, OUTPUT);

  pinMode(output_timer, OUTPUT);
  pinMode(output_timer2, OUTPUT);
  digitalWrite(output_timer, LOW);


  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());




  server.on("/updateopen", HTTP_GET, [](AsyncWebServerRequest * request) {

    for (pos = 0; pos <= 150; pos += 5) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);    // tell servo to go to position in variable 'pos'
      delay(150);             // waits 15ms for the servo to reach the position
    }

    // Add the following to send an HTTP response back to the client
    request->send(200, "text/plain", "OK");
  });

  server.on("/updateclose", HTTP_GET, [](AsyncWebServerRequest * request) {

    for (pos = 150; pos >= 0; pos -= 5) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);    // tell servo to go to position in variable 'pos'
      delay(150);             // waits 15ms for the servo to reach the position
    }

    // Add the following to send an HTTP response back to the client
    request->send(200, "text/plain", "OK");
  });











  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/updatefrontdoors", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputParam;

    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputParam = PARAM_INPUT_1;
      frontdoorsledState = !frontdoorsledState;
    }
    request->send(200, "text/plain", "OK");
  });
  
  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/updatebackdoors", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputParam;

    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputParam = PARAM_INPUT_1;
      backdoorsledState = !backdoorsledState;
    }
    request->send(200, "text/plain", "OK");
  });






// Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1_TIMER)) {
      inputMessage = request->getParam(PARAM_INPUT_1_TIMER)->value();
      digitalWrite(output_timer, inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2_TIMER)) {
      inputMessage = request->getParam(PARAM_INPUT_2_TIMER)->value();
      timerSliderValue = inputMessage;
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });


  



  // Route for index
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route for index
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/symbiot4.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/symbiot4.svg", "image/svg+xml");
  });

  server.on("/icons-sprite.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/icons-sprite.svg", "image/svg+xml");
  });

  server.on("/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bootstrap.bundle.min.js", "text/javascript");
  });

  server.on("/iot-functions.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/iot-functions.min.js", "text/javascript");
  });

  server.on("/iot-range-slider.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/iot-range-slider.min.js", "text/javascript");
  });

  server.on("/iot-timer.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/iot-timer.min.js", "text/javascript");
  });

  server.on("/chartist.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/chartist.min.js", "text/javascripts");
  });

  server.on("/chartist-legend.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/chartist-legend.min.js", "text/javascript");
  });

  server.on("/svg4everybody.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/svg4everybody.min.js", "text/javascript");
  });

  // Route for appliances
  server.on("/appliances.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/appliances.html", String(), false, processor);
  });

  // Route lights
  server.on("/lights.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/lights.html", String(), false, processor);
  });

  // Route for climate
  server.on("/climate.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/climate.html", String(), false, processor);
  });

  // Route for root logout
  server.on("/login.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/login.html", String(), false, processor);
  });


  // Start server
  server.begin();
}

void loop() {
  digitalWrite(frontdoorsledPin, frontdoorsledState);
  digitalWrite(backdoorsledPin, backdoorsledState);
  digitalWrite(output_timer2, LOW);
}
