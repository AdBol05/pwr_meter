#include "api/String.h"
#include <Arduino.h>
#include <Wire.h>
#include <INA226.h>
#include <Adafruit_GFX.h>
#include <DIYables_OLED_SSD1309.h>
//#include <U8g2lib.h>

INA226 ina(0x40);
DIYables_OLED_SSD1309 display(128, 64, &Wire, -1);


int targetTemp = 55;
long int vent_interval = 15000;
int vent_length = 110;
long int offset = 5000;
long int short_interval = 10000;
int short_length = 45;


long int lastVent = 0;
long int lastShort = 0;

int data[6] =       {55, 15000, 110, 5000, 10000, 45};
int increments[6] = {1, 1000, 10, 1000, 1000, 1};

int cursor = 0;

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

void setup1(){
  delay(2000);

  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(PLUS, INPUT_PULLUP);
  pinMode(MINUS, INPUT_PULLUP);
}


void update(){
  if(data[cursor] <= 0){data[cursor] = increments[cursor];}
  if (cursor > 5){cursor = 0;}
  if (cursor < 0){cursor = 5;}
  delay(150);
}

void loop1(){
  if(millis() - lastVent > vent_interval){
    lastVent = millis() - 1;
    digitalWrite(VENT, HIGH);
    delay(vent_length);
    digitalWrite(VENT, LOW);
  }

  if(millis() - lastShort > short_interval){
    lastShort = millis() - 1;
    digitalWrite(SWITCH, LOW);
    delay(3);
    digitalWrite(SHORT, HIGH);
    delay(short_length);
    digitalWrite(SHORT, LOW);
    delay(3);
    digitalWrite(SWITCH, HIGH);
  }

  if(digitalRead(DOWN) == LOW){cursor++; update();}
  if(digitalRead(UP) == LOW){cursor--; update();}
  if(digitalRead(PLUS) == LOW){data[cursor] = data[cursor] + increments[cursor]; update();}
  if(digitalRead(MINUS) == LOW){data[cursor] = data[cursor] - increments[cursor]; update();}
  delay(10);
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
    
    if(cursor <= 3){display.setCursor(0, 25 + (cursor*10));}
    else{display.setCursor(65, 25 + ((cursor - 4)*10));}
    display.print(">");

    display.setCursor(0, 25);
    display.print(" Tt: " + String(data[0]));
    display.setCursor(0, 35);
    display.print(" Vi: " + String(data[1] / 1000) + "s");
    display.setCursor(0, 45);
    display.print(" Vw: " + String(data[2]) + "ms");
    display.setCursor(0, 55);
    display.print(" Ofst: " + String(data[3] / 1000) + "s");
    
    
    display.setCursor(65, 25);
    display.print(" Si: " + String(data[4] / 1000) + "s");
    display.setCursor(65, 35);
    display.print(" Sw: " + String(data[5]) + "ms");

    /*display.setCursor(65, 55);
    display.print(String(cursor));*/
    
    display.display();

    Serial.print(millis());
    Serial.print(", ");
    Serial.print(String(voltage, 2));
    Serial.print(", ");
    Serial.println(String(current, 3));

    if(temp > data[0] + 5){
      if(temp > 80){analogWrite(FAN, 255);}
      else{analogWrite(FAN, 128);}
    }
    else{analogWrite(FAN, 40);}
  
    delay(500);
  }
  else{
    Serial.println("INA INIT FAILED");
    inastatus = ina.begin();
    delay(5000);
  }
}