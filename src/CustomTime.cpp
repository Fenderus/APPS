#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "CustomTime.h"

int second = 0;
int minute = 0;
int hour = 0;
int time = 10;

#define DEFAULT_VALUE 0xFF

CustomTime::CustomTime(int h, int m, int s, int t){
  second = s;
  minute = m;
  hour = h;
  time = t;
}

CustomTime::CustomTime(int order){
  int placement = order * 10;

  byte value = EEPROM.read(placement + 1);
  
  if(value == 255){
    second = 0;
    minute = 0;
    hour = 0;
    time = 10;
    return;
  }

  hour = EEPROM.read(placement + 1);
  minute = EEPROM.read(placement + 2);
  second = EEPROM.read(placement + 3);
  time = EEPROM.read(placement + 4);
}

void CustomTime::setTime(int h, int m, int s, int t){
  second = s;
  minute = m;
  hour = h;
  time = t;
}

void CustomTime::serializeJson(JsonDocument& json, int order) {
  json["hour"] = hour;
  json["minute"] = minute;
  json["second"] = second;
  json["time"] = time;
}

void CustomTime::deserializeJson(JsonDocument& json, int order) {
  hour = json["hour"];
  minute = json["minute"];
  second = json["second"];
  time = json["time"];
}

void CustomTime::saveROM(int order) {
  int placement = order * 10;

  EEPROM.write(placement + 1, hour);
  EEPROM.write(placement + 2, minute);
  EEPROM.write(placement + 3, second);
  EEPROM.write(placement + 4, time);
}