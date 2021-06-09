// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "SPIFFS.h"
#include "creds.h"


// Set LED GPIO
const int frontdoorsledPin = 4;
const int backdoorsledPin = 2;
const char* PARAM_INPUT_1 = "state";

const char* PARAM_INPUT_1_TIMER = "state";
const char* PARAM_INPUT_2_TIMER = "value";
const int output_timer = 21;
String timerSliderValue = "10";

// Stores LED state
int frontdoorsledState = LOW;
int backdoorsledState = LOW;

const int output2 = 4;
String sliderValue = "0";
// setting PWM properties
const int freq = 5000;
const int ledChannel_bedroom = 0;
const int ledChannel_lounge = 1;
const int resolution = 8;
const char* PARAM_INPUT = "value";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Replaces placeholder with button section in web page
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
  if (var == "SLIDERVALUE") {
    return sliderValue;
  }
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    String outputStateValue = outputState();
    buttons+= "<label class=\"switch2\"><input type=\"checkbox\" onchange=\"toggleCheckbox_timer(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  if(var == "TIMERVALUE"){
    return timerSliderValue;
  }
  return String();
}


String outputState(){
  if(digitalRead(output_timer)){
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
  pinMode(frontdoorsledPin, OUTPUT);
  pinMode(backdoorsledPin, OUTPUT);

  pinMode(output_timer, OUTPUT);
  digitalWrite(output_timer, LOW);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }


  // configure LED PWM functionalitites
  ledcSetup(ledChannel_bedroom, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(output2, ledChannel_lounge);
  ledcWrite(ledChannel_lounge, sliderValue.toInt());


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());



  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/loungeslider", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      ledcWrite(ledChannel_lounge, sliderValue.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
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
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
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
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
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

}
