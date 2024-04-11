#ifndef CustomTime_h
#define CustomTime_h

#include <Arduino.h>
#include <ArduinoJson.h>

class CustomTime{
  public:
    CustomTime(int h, int m, int s, int t);
    CustomTime(int order);
    int second;
    int minute;
    int hour;
    int time;
    void setTime(int h, int m, int s, int t);
    void serializeJson(JsonDocument& json, int order);
    void deserializeJson(JsonDocument& json, int order);
    void saveROM(int order);
  private:
};

#endif 