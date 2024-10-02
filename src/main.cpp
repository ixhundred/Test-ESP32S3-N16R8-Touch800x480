#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  EEPROM.begin(4096);
  Serial.printf("#S0 PSRAM Free = %lu/%lu kBytes\r\n",ESP.getFreePsram()/1024,ESP.getPsramSize()/1024);
  Serial1.printf("#S1 PSRAM Free = %lu/%lu kBytes\r\n",ESP.getFreePsram()/1024,ESP.getPsramSize()/1024);
  Serial2.printf("#S2 PSRAM Free = %lu/%lu kBytes\r\n",ESP.getFreePsram()/1024,ESP.getPsramSize()/1024);
}

void loop()
{

}