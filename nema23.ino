#include <AccelStepper.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// Stepper motors
AccelStepper stepper1(1, 11, 12); // Door
AccelStepper stepper2(1, 9, 10);  // Horizontal
AccelStepper stepper3(1, 22, 8);  // Vertical

// TFT display
#define TFT_DC 4
#define TFT_CS 2
#define TFT_RST 3
#define TFT_MISO 50
#define TFT_MOSI 51
#define TFT_CLK 52
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// LDRs
#define LDR1 A0
#define LDR2 A1
#define LDR3 A2
#define LDR4 A3

// Wind sensor
#define WIND_SENSOR A4
const float maxWindVoltage = 5.0;
const float maxWindSpeed = 70;
const float windThreshold = 10.0;

int ldr1, ldr2, ldr3, ldr4;
float windVoltage = 0.0, windSpeed = 0.0;
String action = "";
bool hasLocked = false;

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  stepper1.setMaxSpeed(2000); stepper1.setAcceleration(500); stepper1.setCurrentPosition(0);
  stepper2.setMaxSpeed(3000); stepper2.setAcceleration(500); stepper2.setCurrentPosition(0);
  stepper3.setMaxSpeed(3000); stepper3.setAcceleration(500); stepper3.setCurrentPosition(0);
}

void display(String source, String direction, String state, float wind) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);
  tft.print("Trigger: "); tft.println(source);
  tft.print("Direction: "); tft.println(direction);
  tft.print("Wind: "); tft.print(wind); tft.println(" m/s");
  tft.print("Action: "); tft.println(state);
}

String getLDRDirection() {
  int maxVal = max(max(ldr1, ldr2), max(ldr3, ldr4));
  if (maxVal == ldr1) return "TOP-LEFT";
  else if (maxVal == ldr2) return "TOP-RIGHT";
  else if (maxVal == ldr3) return "BOTTOM-LEFT";
  else return "BOTTOM-RIGHT";
}

bool lightDetected() {
  return (ldr1 > 200 || ldr2 > 200 || ldr3 > 200 || ldr4 > 200);
}

void rotateTo(String dir) {
  if (dir == "TOP-LEFT") {
    stepper3.moveTo(-200); stepper3.runToPosition();
    stepper2.moveTo(-200); stepper2.runToPosition();
  } else if (dir == "TOP-RIGHT") {
    stepper3.moveTo(-200); stepper3.runToPosition();
    stepper2.moveTo(200); stepper2.runToPosition();
  } else if (dir == "BOTTOM-LEFT") {
    stepper3.moveTo(200); stepper3.runToPosition();
    stepper2.moveTo(-200); stepper2.runToPosition();
  } else if (dir == "BOTTOM-RIGHT") {
    stepper3.moveTo(200); stepper3.runToPosition();
    stepper2.moveTo(200); stepper2.runToPosition();
  }
}

void openAndCloseDoor(String dir, String triggerSource) {
  rotateTo(dir);

  action = "Opening Door";
  display(triggerSource, dir, action, windSpeed);
  stepper1.moveTo(800);
  stepper1.runToPosition();  // <<< Open door

  delay(1000); // Keep door open for a moment

  action = "Closing Door";
  display(triggerSource, dir, action, windSpeed);
  stepper1.moveTo(0);
  stepper1.runToPosition();  // <<< Close door

  action = "Locked";
  display(triggerSource, dir, action, windSpeed);
}

void loop() {
  // Read LDRs
  ldr1 = analogRead(LDR1);
  ldr2 = analogRead(LDR2);
  ldr3 = analogRead(LDR3);
  ldr4 = analogRead(LDR4);

  // Read wind sensor
  int windRaw = analogRead(WIND_SENSOR);
  windVoltage = (windRaw / 1023.0) * 5.0;
  windSpeed = (windVoltage / maxWindVoltage) * maxWindSpeed;

  String ldrDir = getLDRDirection();
  String windDir = "BOTTOM-RIGHT"; // Default wind direction if no light

  if (!hasLocked) {
    if (lightDetected()) {
      hasLocked = true;
      openAndCloseDoor(ldrDir, "Light");
    } else if (windSpeed >= windThreshold) {
      hasLocked = true;
      openAndCloseDoor(windDir, "Wind");
    } else {
      action = "Idle";
      display("None", "None", action, windSpeed);
    }
  }
   Serial.println(ldr1);   
    Serial.println(ldr2);   
     Serial.println(ldr3);   
      Serial.println(ldr4);   

  delay(500);
}
