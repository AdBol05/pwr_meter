#include "api/String.h"
#include <Arduino.h>
#include <Wire.h>
#include <INA226.h>
#include <Adafruit_GFX.h>
#include <DIYables_OLED_SSD1309.h>
//#include <U8g2lib.h>

INA226 ina(0x40);
DIYables_OLED_SSD1309 display(128, 64, &Wire, -1);

long int short_interval = 10000;
long int vent_interval = 15000;
long int offset = 5000;

int targetTemp = 55;

int short_length = 45;
int vent_length = 110;

long int lastVent = 0;
long int lastShort = 0;

#define VENT 6
#define SHORT 7
#define SWITCH 8
#define TEMP 26
#define FAN 14

#define PLUS 10
#define MINUS 11
#define UP 12
#define DOWN 13

bool inastatus = false;

#define R_NTC 10000.0
#define R0_NTC 10000.0
#define T0_NTC 298.15
#define BETA_NTC 3950.0

float readVoltage(int pin) {return (analogRead(pin) * 3.3) / 4095;}
float readNTC(int pin) {
  float r = R_NTC * (3.3 / readVoltage(pin) - 1.0);
  return (1.0 / ((1.0 / T0_NTC) + (1.0 / BETA_NTC) * log(r / R0_NTC))) - 273.15;
}

void setup() {
  Wire.setSDA(28); //20
  Wire.setSCL(29); //21
  Wire.begin();

  analogReadResolution(12);

  pinMode(VENT, OUTPUT);
  pinMode(SHORT, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(SWITCH, OUTPUT);

  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(PLUS, INPUT_PULLUP);
  pinMode(MINUS, INPUT_PULLUP);

  /*Wire.write(0x0F);
  Wire.write(0x01);*/
  Wire.endTransmission();

  inastatus = ina.begin();
  ina.setMaxCurrentShunt(8.0, 0.01);

  Serial.begin(115200);
//  Serial.println("oled init...");
  
  if(!display.begin(SSD1309_SWITCHCAPVCC, 0x3C)){/*Serial.println("Failed to initialize oled");*/}
  display.clearDisplay();
  
  analogWrite(FAN, 255);
  digitalWrite(VENT, LOW);
  digitalWrite(SHORT, LOW);
  delay(100);
  digitalWrite(SWITCH, HIGH);

  Serial.println("Time, Voltage, Current");
}

void loop() {
  if(inastatus){
    float voltage = ina.getBusVoltage();
    float current = ina.getCurrent();
    float temp = readNTC(TEMP);
  
    /*if(voltage < 6.5){digitalWrite(19, LOW); delay(3000);}
    else {digitalWrite(19, HIGH);}*/

    display.clearDisplay();
    display.setTextSize(1);                  // 6x8 pixels per character
    display.setTextColor(SSD1309_PIXEL_ON);  // turn pixels on
    display.setCursor(15, 0);
    display.print(String(voltage, 2) + " V");
    display.setCursor(65, 0);
    display.print(String(current, 3) + " A");
    display.setCursor(15, 10);
    display.print(String(temp, 1) + " C");
    display.setCursor(65, 10);
    display.print(String(current*voltage, 3) + " W");
    display.setCursor(0, 17);
    display.print("---------------------");
    
    display.setCursor(0, 25);
    display.print(" Tt: " + String(targetTemp));
    display.setCursor(0, 35);
    display.print(" Vi: " + String(vent_interval / 1000) + "s");
    display.setCursor(0, 45);
    display.print(" Vw: " + String(vent_length) + "ms");
    display.setCursor(0, 55);
    display.print(" Ofst: " + String(offset / 1000) + "s");
    
    
    display.setCursor(65, 25);
    display.print(" Si: " + String(short_interval / 1000) + "s");
    display.setCursor(65, 35);
    display.print(" Sw: " + String(short_length) + "ms");
    
    display.display();

    Serial.print(millis());
    Serial.print(", ");
    Serial.print(String(voltage, 2));
    Serial.print(", ");
    Serial.println(String(current, 3));
  
    delay(500);
  }
  else{
    Serial.println("INA INIT FAILED");
    inastatus = ina.begin();
    delay(5000);
  }
}