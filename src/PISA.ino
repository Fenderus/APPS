#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <List.hpp>
#include "CustomTime.h"
#include <EEPROM.h>

#define LED_TEST_PIN 7
#define UP_TEST_PIN 15
#define DOWN_TEST_PIN 14
#define LEFT_TEST_PIN 17
#define RIGHT_TEST_PIN 16
#define REJECT_TEST_PIN 8
#define ACCEPT_TEST_PIN 9

ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address and dimensions

//EditPage edit(14,15, lcd);

//CustomTime defaultTime(20, 19, 50, 10);
const int schedLength = 10;
CustomTime scheds[schedLength] = {CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10),CustomTime(0,0,0,10)};

//List<CustomTime> schedules(false);

typedef enum Pages{
  TIME,
  NEW_SCHEDULE,
  LIST_SCHEDULES,
  RESET_CONF
};

const int buttonPin = 2; // Define the button pin
int buttonState = 0; // Variable for reading the button state
bool isHelloWorld = true; // Variable to track the current state
int defaultTime = 10;
int currentTime = 10;
bool timeOn = false;
bool militaryTime = true;
int currentSecond;
int currentMinute;
int currentHour;
int currentTimer = 10;
int currentNum = 1;
Pages currentPage;
int accept_state = HIGH;
int prev_accept_state = HIGH;
int reject_state = HIGH;
int prev_reject_state = HIGH;
int mills = 100;
int currentInterv = 0;
int currentSelectedSchedule = 0;
String currentFirstText;
String currentSecondText;

unsigned long prevMillis = millis();


void setup() {
  Serial.begin(9600);

  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight

  lcd.setCursor(0, 0); // Set cursor to the first column of the first row
  lcd.print("Sage Cheecks"); // Display "Hello World" on startup

  
  pinMode(LED_TEST_PIN, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(UP_TEST_PIN, INPUT_PULLUP);
  pinMode(DOWN_TEST_PIN, INPUT_PULLUP);
  pinMode(LEFT_TEST_PIN, INPUT_PULLUP);
  pinMode(RIGHT_TEST_PIN, INPUT_PULLUP);
  pinMode(REJECT_TEST_PIN, INPUT_PULLUP);
  pinMode(ACCEPT_TEST_PIN, INPUT_PULLUP);
  
   // Set the button pin as input

  //currentTime = defaultTime.time;
  
  setupRTC();
  read();
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  //Serial.println(String(defaultTime.second));

  // button states:
  firstState();

  //displayTime(now);
  //edit.updating();
  //lcd_Place();

  

  if (currentPage == TIME) {
    showTime(now);
  }else if (currentPage == NEW_SCHEDULE) {
    lcd_Place();
  }else if (currentPage == LIST_SCHEDULES){
    showSchedules();
  }else if (currentPage == RESET_CONF){
    resetConfirmation();
  }


  checkTime(now);

  if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }


  lcd_Time();
  //Serial.println(String(accept_state) + " " + String(prev_accept_state));


  delay(mills);
  currentInterv += mills;
  if(currentInterv >= (1000)){
    currentInterv = 0;
  }

  //Serial.println(String(currentInterv));

  // Prev Button States
  lastState();
}

void setupRTC(){
    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
        //Rtc.SetDateTime(compiled);
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void displayMessage(String line1, String line2) {
  lcd.clear(); // Clear the LCD screen
  lcd.setCursor(0, 0); // Set cursor to the first column of the first row
  lcd.print(line1); // Display line1 on the first row
  lcd.setCursor(0, 1); // Set cursor to the first column of the second row
  lcd.print(line2); // Display line2 on the second row
}

void displayMessage(String line, int cursor, bool secondInterv, bool checkers) {
  if(secondInterv){
    if(!(currentInterv <= 0)){
      return;
    }
  }

  if (checkers){
    if(cursor == 0){
      if(line == currentFirstText){
        //Serial.println()
        return;
      }
    }else if(cursor == 1){
      if(line == currentSecondText){
        return;
    }
    }
  }

  lcd.setCursor(0, cursor); // Set cursor to the first column of the first row
  lcd.print("                "); // Clearing the line by printing spaces

  // Write something on the cleared line
  lcd.setCursor(0, cursor);
  lcd.print(line);

  if(cursor == 0){
    currentFirstText = line;
  }else if(cursor == 1){
    currentSecondText = line;
  }
}

void clearLine(int cursor){
  displayMessage("           ", cursor, false, true);
}

void firstState(){
  accept_state = digitalRead(ACCEPT_TEST_PIN);
  reject_state = digitalRead(REJECT_TEST_PIN);
}

void lastState(){
  prev_accept_state = accept_state;
  prev_reject_state = reject_state;
}

void checkTime(const RtcDateTime& dt){
  int arraySize = (sizeof(scheds) / sizeof(scheds[0]));

  for (int i = 0; i < arraySize; i++)
  {
    if(dt.Hour() == scheds[i].hour && dt.Minute() == scheds[i].minute && dt.Second() == scheds[i].second){
      //Serial.println("Same Time");
      currentTime = scheds[i].time;
      timeOn = true;
    } else{
      //Serial.println("Not Time");
    }
  }

  //Serial.println(String(arraySize) + " is the size of the array");
}

void lcd_Time(){
  unsigned long currentMillis = millis();
  
  if (currentMillis - prevMillis > 1000){
    if(timeOn == true){
      currentTime -= 1;
      digitalWrite(LED_TEST_PIN, HIGH);
      if (currentPage == TIME) {
        displayMessage("Pump on: " + String(currentTime + 1), 1, false, true);
      }
      if(currentTime < 0){
        clearLine(0);
        timeOn = false;
        currentTime = 10;
        digitalWrite(LED_TEST_PIN, LOW);
      }
    }

    prevMillis = currentMillis;
  }

  //Serial.println(currentTime);
  //Serial.println(String(timeOn));
}

void lcd_Place(){
  if(currentNum == 1){
    if(digitalRead(DOWN_TEST_PIN) == HIGH){
      currentHour += 1;
      if(currentHour > 23){
        currentHour = 0;
      }
    }

    if(digitalRead(UP_TEST_PIN) == HIGH){
      currentHour -= 1;
      if(currentHour < 0){
        currentHour = 23;
      }
    }
  }

  if(currentNum == 2){
    if(digitalRead(DOWN_TEST_PIN) == HIGH){
      currentMinute += 1;
      if(currentMinute > 59){
        currentMinute = 0;
      }
    }

    if(digitalRead(UP_TEST_PIN) == HIGH){
      currentMinute -= 1;
      if(currentMinute < 0){
        currentMinute = 59;
      }
    }
  }

  if(currentNum == 3){
    if(digitalRead(DOWN_TEST_PIN) == HIGH){
      currentSecond += 1;
      if(currentSecond > 59){
        currentSecond = 0;
      }
    }

    if(digitalRead(UP_TEST_PIN) == HIGH){
      currentSecond -= 1;
      if(currentSecond < 0){
        currentSecond = 59;
      }
    }
  }

  if(currentNum == 4){
    if(digitalRead(DOWN_TEST_PIN) == HIGH){
      currentTimer += 1;
    }

    if(digitalRead(UP_TEST_PIN) == HIGH){
      currentTimer -= 1;
      if(currentTimer < 0){
        currentTimer = 0;
      }
    }
  }

  int lefttistate = digitalRead(LEFT_TEST_PIN);
  int righttistate = digitalRead(RIGHT_TEST_PIN);

  if(righttistate == HIGH){
    currentNum -= 1;

    if(currentNum <= 0){
      currentNum = 4;
    }
  }

  if(lefttistate == HIGH){
    currentNum += 1;

    if(currentNum > 4){
      currentNum = 1;
    }
  }

  char timeString[12]; // Allocate space for 8 characters plus null terminator
  sprintf(timeString, "%02d:%02d:%02d>%02d", currentHour, currentMinute, currentSecond, currentTimer); // Format hours, minutes, and seconds into the string

  //lcd.clear(); // Clear the LCD screen
  //lcd.setCursor(0, 0); // Set cursor to the first column of the first row
  displayMessage(timeString, 0, false, true);
 // Clear the LCD screen
  //lcd.setCursor(0, 1); // Set cursor to the first column of the first row
  if(currentNum == 1){
    displayMessage(" ^", 1, false, true);
  }
  if(currentNum == 2){
    displayMessage("    ^", 1, false, true);
  }
  if(currentNum == 3){
    displayMessage("       ^", 1, false, true);
  }
  if(currentNum == 4){
    displayMessage("          ^", 1, false, true);
  }

  //int acc = digitalRead(ACCEPT_TEST_PIN);

  if(accept_state == HIGH && prev_accept_state == LOW){
    currentPage = LIST_SCHEDULES;
    //defaultTime.setTime(currentHour, currentMinute, currentSecond, currentTimer);

    scheds[currentSelectedSchedule] = (CustomTime(currentHour, currentMinute, currentSecond, currentTimer));
  
    currentTime = scheds[currentSelectedSchedule].time;
    save(currentSelectedSchedule);
    //Serial.println("Pressed");
  }

  if(reject_state == HIGH && prev_reject_state == LOW){
    currentPage = LIST_SCHEDULES;
    //defaultTime.setTime(currentHour, currentMinute, currentSecond, currentTimer);
    //Serial.println("Pressed");
  }
}

void showTime(const RtcDateTime& dt){

  int hour = 0;
  String side = "AM";
  if(militaryTime){
    hour = dt.Hour();
    side = "";
  }else{
    if(dt.Hour() > 12){
      hour = dt.Hour() - 12;
      side = "pm";
    }else{
      side = "am";
    }
  }

  char timeString[9]; // Allocate space for 8 characters plus null terminator
    sprintf(timeString, "%02d:%02d:%02d", hour, dt.Minute(), dt.Second()); // Format hours, minutes, and seconds into the string

  displayMessage("Time: " + String(timeString) + side, 0, false, true);
  if (!timeOn)
  {
    clearLine(1);
  }
  
  int lefttistate = digitalRead(LEFT_TEST_PIN);
  int righttistate = digitalRead(RIGHT_TEST_PIN);

  int upState = digitalRead(UP_TEST_PIN);
  int downState = digitalRead(DOWN_TEST_PIN);

  if(upState == LOW || downState == LOW){
    militaryTime = !militaryTime;
    //Serial.println("Pressed");
  }

  if(righttistate == LOW || lefttistate == LOW){
    currentPage = LIST_SCHEDULES;
    //Serial.println("Pressed");
  }

  if(accept_state == HIGH && prev_accept_state == LOW){
    if(!timeOn){
      currentTime = defaultTime;
      timeOn = true;
    }
  }
}

void showSchedules(){
  int arraySize = (sizeof(scheds) / sizeof(scheds[0]));

  int hour1 = hour1 = scheds[currentSelectedSchedule].hour;
  int hour2 = hour1 = scheds[currentSelectedSchedule+1].hour;
  String side1 = "AM";
  String side2 = "AM";
  if(militaryTime){
    hour1 = scheds[currentSelectedSchedule].hour;
    side1 = "";
    hour2 = scheds[currentSelectedSchedule+1].hour;
    side2 = "";
  }else{
    if(scheds[currentSelectedSchedule].hour > 12){
      hour1 = scheds[currentSelectedSchedule].hour - 12;
      side1 = "pm";
    }else{
      hour1 = scheds[currentSelectedSchedule].hour;
      side1 = "am";
    }

    if(scheds[currentSelectedSchedule+1].hour > 12){
      hour2 = scheds[currentSelectedSchedule+1].hour - 12;
      side2 = "pm";
    }else{
      hour2 = scheds[currentSelectedSchedule+1].hour;
      side2 = "am";
    }
  }

  char firstSelected[12]; // Allocate space for 8 characters plus null terminator
  sprintf(firstSelected, "%02d:%02d:%02d>%02d", hour1, scheds[currentSelectedSchedule].minute, scheds[currentSelectedSchedule].second, scheds[currentSelectedSchedule].time);

  displayMessage(String(currentSelectedSchedule + 1) + "." + String(firstSelected) + String(side1) + "<", 0, false, true);

  if (currentSelectedSchedule != (schedLength-1))
  {
    char secondSelected[12]; // Allocate space for 8 characters plus null terminator
    sprintf(secondSelected, "%02d:%02d:%02d>%02d", hour2, scheds[currentSelectedSchedule + 1].minute, scheds[currentSelectedSchedule + 1].second, scheds[currentSelectedSchedule + 1].time);
    displayMessage(String(currentSelectedSchedule + 2) + "." +String(secondSelected) + side2, 1, false, true);
    //displayMessage(String(currentSelectedSchedule + 2) + ". " +String(secondSelected), 1, false, true);
  }else{
    displayMessage("           ", 1, false, true);
  }

  int lefttistate = digitalRead(LEFT_TEST_PIN);
  int righttistate = digitalRead(RIGHT_TEST_PIN);
  int upState = digitalRead(UP_TEST_PIN);
  int downState = digitalRead(DOWN_TEST_PIN);

  if(downState == LOW){
    currentSelectedSchedule += 1;
    if(currentSelectedSchedule > (schedLength-1)){
      currentSelectedSchedule = 0;
    }
  }

  if(upState == LOW){
    currentSelectedSchedule -= 1;
    if(currentSelectedSchedule < 0){
      currentSelectedSchedule = (schedLength-1);
    }
  }

  if(righttistate == LOW || lefttistate == LOW){
    currentPage = TIME;
    //Serial.println("Pressed");
  }

  if(accept_state == HIGH && prev_accept_state == LOW){
    currentPage = NEW_SCHEDULE;
    //Serial.println("Pressed");
  }

  if(reject_state == HIGH && prev_reject_state == LOW){
    //clearAll();
    //read();
    currentPage = RESET_CONF;
  }
}

void resetConfirmation(){
  displayMessage("Confirm to clear", 0, false, true);
  displayMessage("All schedules?", 1, false, true);

  if(accept_state == HIGH && prev_accept_state == LOW){
    clearAll();
    read();
    currentPage = LIST_SCHEDULES;
  }

  if(reject_state == HIGH && prev_reject_state == LOW){
    currentPage = LIST_SCHEDULES;
    //Serial.println("Pressed");
  }
}

void save(int order){
  scheds[order].saveROM(order);
}

void read(){
  for (int i = 0; i < schedLength; i++)
  {
    scheds[i] = CustomTime(i);
  }
}

void clearAll(){
  // Clearing EEPROM
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }

  // Print message
  Serial.println("EEPROM Cleared!");

}