#include <Arduino.h>
#include <Wire.h>
#include <INA226.h>

INA226 ina(0x40);

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

void setup() {
  Wire.setSDA(28); //20
  Wire.setSCL(29); //21
  Wire.begin();

  pinMode(19, OUTPUT);

  /*Wire.write(0x0F);
  Wire.write(0x01);*/
  Wire.endTransmission();

  inastatus = ina.begin();
  ina.setMaxCurrentShunt(8.0, 0.01);

  Serial.begin(115200);
  Serial.println("Time, Voltage, Current");

  digitalWrite(19, HIGH);
}

void loop() {
  if(inastatus){
    float voltage = ina.getBusVoltage();
    float current = ina.getCurrent();
  
    if(voltage < 6.5){digitalWrite(19, LOW); delay(3000);}
    else {digitalWrite(19, HIGH);}
  
    Serial.print(millis());
    Serial.print(", ");
    Serial.print(String(voltage, 2));
    Serial.print(", ");
    Serial.println(String(current, 3));
  
    delay(10);
  }
  else{
    Serial.println("INA INIT FAILED");
    inastatus = ina.begin();
    delay(5000);
  }
}