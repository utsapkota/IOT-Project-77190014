// Import required libraries

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <SPIFFS.h>
#include <ESP32_MailClient.h>
#include "creds.h"

#define timeSeconds 3
#define emailSenderAccount    "utsav17fun@gmail.com"    // Sender email address
#define emailSenderPassword   "uSAP199742"            // Sender email password
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "ALERT! Gas Leak Detected"   // Email subject

String inputMessage = "Email Recipient";   //Reciepent email alert.
String enableEmailChecked = "checked";
String inputMessage2 = "true";

// Default Threshold Value
String inputMessage3 = "100";                    // Default gas_value
String lastgaslevel;

const int buzzer = 18;
const int motionSensor = 27;
int buzzerState = LOW;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

int pos = 0;
Servo myservo;

// Set LED GPIO
const int frontdoorsledPin = 2;
const int backdoorsledPin = 23;
const char* PARAM_INPUT_1 = "state";
int frontdoorsledState = LOW;
int backdoorsledState = LOW;




int r, b, g;
// Red, green, and blue pins for PWM control
const int redPin = 13;     // 13 corresponds to GPIO13
const int greenPin = 12;   // 12 corresponds to GPIO12
const int bluePin = 14;    // 14 corresponds to GPIO14

// Setting PWM frequency, channels and bit resolution
const int freq_led = 5000;
const int redChannel = 4;
const int greenChannel = 5;
const int blueChannel = 6;
// Bit resolution 2^8 = 256
const int resolution_led = 8;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;





const char* PARAM_INPUT_1_TIMER = "state";
const char* PARAM_INPUT_2_TIMER = "value";
String timerSliderValue = "10";
const int output_timer = 32;
const int output_timer2 = 33;


Adafruit_BMP085 bmp;


AsyncWebServer server(80);
String processor(const String& var) {
  //Serial.println(var);
  if (var == "FRONTDOOR") {
    String buttons = "";
    buttons += "<label class=\"switch ml-auto\"><input type=\"checkbox\" id=\"switch-light-6\" onchange=\"toggleFrontdoor(this)\"></label>";
    return buttons;
  }
  if (var == "BACKDOOR") {
    String buttons = "";
    buttons += "<label class=\"switch ml-auto\"><input type=\"checkbox\" id=\"switch-light-7\" onchange=\"toggleBackdoor(this)\"></label>";
    return buttons;
  }
  if (var == "LOUNGEBUTTON") {
    String buttons = "";
    buttons += "<label class=\"switch ml-auto\"><input type=\"checkbox\" id=\"switch-light-2\" onchange=\"toggleLounge(this)\"></label>";
    return buttons;
  }
  if (var == "OPENPLACEHOLDER") {
    String buttons = "";
    buttons += "<button data-action=\"open\" type=\"button\" class=\"btn btn-secondary doors-control\" onclick=\"toggleOpen(this)\">Open</button>";
    return buttons;
  }
  if (var == "CLOSEPLACEHOLDER") {
    String buttons = "";
    buttons += "<button data-action=\"close\" type=\"button\" class=\"btn btn-secondary doors-control\" onclick=\"toggleClose(this)\">Close</button>";
    return buttons;
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
  if (var == "SECURITY") {
    String buttons = "";
    buttons += "<label class=\"switch ml-auto\"><input type=\"checkbox\" id=\"switch-house-lock\" onchange=\"toggleSecurity(this)\"></label>";
    return buttons;
  }
  if (var == "GASVALUE")
  {
    return lastgaslevel;
  }
  if (var == "EMAIL_INPUT")
  {
    return inputMessage;
  }
  if (var == "ENABLE_EMAIL")
  {
    return enableEmailChecked;
  }
  if (var == "THRESHOLD")
  {
    return inputMessage3;
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





// Flag variable to keep track if email notification was sent or not
bool emailSent = false;
const char* PARAM_INPUT_1_EMAIL = "email_input";
const char* PARAM_INPUT_2_EMAIL = "enable_email_input";
const char* PARAM_INPUT_3_EMAIL = "threshold_input";

// Interval between sensor readings.
unsigned long previousMillis = 0;
const long interval = 5000;

SMTPData smtpData;



String readBMP180Temperature() {
  // Read temperature as Celsius (the default)
  float t = bmp.readTemperature();
  // Convert temperature to Fahrenheit
  //t = 1.8 * t + 32;
  if (isnan(t)) {
    Serial.println("Failed to read from BMP180 sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readBMP180Altitude() {
  float h = bmp.readAltitude();
  if (isnan(h)) {
    Serial.println("Failed to read from BMP180 sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readBMP180Pressure() {
  float p = bmp.readPressure() / 100.0F;
  if (isnan(p)) {
    Serial.println("Failed to read from BMP180 sensor!");
    return "";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}



void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}


// Checks if motion was detected, sets buzzer HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  digitalWrite(buzzer, buzzerState);
  startTimer = true;
  lastTrigger = millis();
}


void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  myservo.attach(25);

  pinMode(frontdoorsledPin, OUTPUT);
  pinMode(backdoorsledPin, OUTPUT);



  // configure LED PWM functionalitites
  ledcSetup(redChannel, freq_led, resolution_led);
  ledcSetup(greenChannel, freq_led, resolution_led);
  ledcSetup(blueChannel, freq_led, resolution_led);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(redPin, redChannel);
  ledcAttachPin(greenPin, greenChannel);
  ledcAttachPin(bluePin, blueChannel);




  pinMode(output_timer, OUTPUT);
  pinMode(output_timer2, OUTPUT);
  digitalWrite(output_timer, LOW);


  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set buzzer to LOW
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);


  bool status;
  status = bmp.begin();
  if (!status) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1);
  }



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







  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/updatesecurity", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputParam;

    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputParam = PARAM_INPUT_1;
      buzzerState = !buzzerState;
      digitalWrite(buzzer, LOW);
    }
    request->send(200, "text/plain", "OK");
  });








  // Receive an HTTP GET request at <ESP_IP>/get?email_input=<inputMessage>&enable_email_input=<inputMessage2>&threshold_input=<inputMessage3>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1_EMAIL)) {
      inputMessage = request->getParam(PARAM_INPUT_1_EMAIL)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2_EMAIL)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2_EMAIL)->value();
        enableEmailChecked = "checked";
      }
      else
      {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
      // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
      if (request->hasParam(PARAM_INPUT_3_EMAIL)) {
        inputMessage3 = request->getParam(PARAM_INPUT_3_EMAIL)->value();
      }
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    Serial.println(inputMessage3);
    request->send(SPIFFS, "/appliances.html", String(), false, processor);
  });




  server.on("/climate.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/climate.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBMP180Temperature().c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBMP180Altitude().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBMP180Pressure().c_str());
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
    int parameter_numbers = request->params();
    int arr[3] = {0, 0, 0};
    for (int i = 0; i < parameter_numbers; i++) {

      AsyncWebParameter* p = request->getParam(i);
      arr[i] = (p->value()).toInt();
    }
    Serial.println(arr[0]);
    Serial.println(arr[1]);
    Serial.println(arr[2]);
    r = arr[0];
    g = arr[1];
    b = arr[2];
    request->send(SPIFFS, "/lights.html", String(), false, processor);
  });

  // Route for root logout
  server.on("/login.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/login.html", String(), false, processor);
  });


  server.onNotFound(notFound);
  // Start server
  server.begin();
}

void loop() {
  digitalWrite(frontdoorsledPin, frontdoorsledState);
  digitalWrite(backdoorsledPin, backdoorsledState);
  digitalWrite(output_timer2, LOW);

  ledcWrite(redChannel, r);
  ledcWrite(greenChannel, g);
  ledcWrite(blueChannel, b);

  // Current time
  now = millis();
  // Turn off the buzzer after the number of seconds defined in the timeSeconds variable
  if (startTimer && (now - lastTrigger > (timeSeconds * 1000)))
  {
    Serial.println("Motion stopped...");
    digitalWrite(buzzer, buzzerState);
    startTimer = false;
  }





  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float gas_analog_value = analogRead(35);
    float gas_value = ((gas_analog_value / 1023) * 100);
    Serial.print(gas_analog_value);
    Serial.print(", ");
    Serial.println(gas_value);

    lastgaslevel = String(gas_value);

    // Check if gas_value is above threshold and if it needs to send the Email alert
    if (gas_value > inputMessage3.toFloat() && inputMessage2 == "true" && !emailSent) {
      String emailMessage = String("Gas Level above threshold. Current Gas Level: ") + String(gas_value);
      if (sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }
    }
    // Check if gas_value is below threshold and if it needs to send the Email alert
    else if ((gas_value < inputMessage3.toFloat()) && inputMessage2 == "true" && emailSent)
    {
      String emailMessage = String("Gas Level below threshold. Current Gas Level: ") + String(gas_value);
      if (sendEmailNotification(emailMessage))
      {
        Serial.println(emailMessage);
        emailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }
  }
}






bool sendEmailNotification(String emailMessage)
{
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  smtpData.setSender("URGENT - Home Gas Leak Alert", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);
  smtpData.setSendCallback(sendCallback);

  if (!MailClient.sendMail(smtpData))
  {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }

  smtpData.empty();
  return true;
}


void sendCallback(SendStatus msg)
{
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success())
  {
    Serial.println("----------------");
  }
}
