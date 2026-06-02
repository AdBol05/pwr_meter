#include <Arduino.h>
#include <Wire.h>
#include <INA226.h>

INA226 ina(0x40);

void setup() {
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  /*Wire.write(0x0F);
  Wire.write(0x01);*/
  Wire.endTransmission();

  ina.begin();
  ina.setMaxCurrentShunt(8.0, 0.01);

  Serial.begin(115200);
  Serial.println("Voltage, Current");
}

void loop() {
  float voltage = ina.getBusVoltage();
  float current = ina.getCurrent();

  Serial.print(String(voltage, 2));
  Serial.print(", ");
  Serial.println(String(current, 3));

  delay(10);
}