/******************************************************************
 Project     :  Aromatron v1
 Author      :  Leo Keil
 Description :  Melitta Aromaboy automation to make Coffee every morning automatically.
******************************************************************/

/*************************************************
++++++++++++[DEFINITIONS]+++++++++++++++++++++++++
*************************************************/
//---------- [Libraries]----------
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Checksum.h>
#include <EEPROM-Storage.h>
#include "customCharacters.h"

//----------[Pins]----------
#define pinRotIn1 2
#define pinRotIn2 3
#define pinRotBtn 4
#define pinBuzzer 5
#define pinArmBtn 6
#define pinRedLed 7
#define pinGreenLed 8
#define pinRelais 12

//----------[LCD]----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

//----------[RTC]----------
RTC_DS3231 rtc;

/*************************************************
++++++++++++[VARIABLES]+++++++++++++++++++++++++++
*************************************************/
//General
bool isArmed;
int menuState;

//LCD
bool clearflag;

//Time
DateTime now;
DateTime prevNow;
char daysOfTheWeek[7][12] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

//Rotary Encoder
int rotaryLastState;

/*************************************************
++++++++++++[SETUP]+++++++++++++++++++++++++++++++
*************************************************/
void setup()
{
  //Initialize Debug Serial Connection
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Aromatron Debug Serial Begin:");
  Serial.println("[INFO] Setup started.");
  
  //Pin Setup
  pinMode(pinRotIn1, INPUT);
  pinMode(pinRotIn2, INPUT);
  pinMode(pinRotBtn, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinArmBtn, INPUT_PULLUP);
  pinMode(pinRedLed, OUTPUT);
  pinMode(pinGreenLed, OUTPUT);
  pinMode(pinRelais, OUTPUT);

  //Disable Relais instantly
  changeRelais(false);

  //Initialize LCD
  lcd.init();
  lcd.noAutoscroll();
  lcd.createChar(0, charCoffeeHeart);
  lcd.createChar(1, charCoffeeArmed);
  lcd.createChar(2, charCoffeeDisarmed);
  lcd.createChar(3, charBean);
  lcd.createChar(4, charRestart);
  lcd.createChar(5, charWait);
  lcd.createChar(6, charClock);
  lcd.createChar(7, charCalendar);
  lcd.backlight();

  //Initialize RTC
  //try to start RTC
  if (!rtc.begin())
  {
    //throw error when rtc isn't connected
    showError("RTC NOT FOUND", true);
  }

  //Update the rtc with compile time date/time
  //TODO change this to something 
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  //check if rtc lost power
  if (rtc.lostPower())
  {
    //throw error when rtc lost power
    showError("RTC LOST POWER", false);

    //open time input dialogue
    //TODO
  }

  //Initialize rotary encoder logic
  rotaryLastState = digitalRead(pinRotIn1);

  Serial.println("[INFO] Setup completed.");

  //----------POST SETUP----------
  //LCD: print bootscreen
  lcd.setCursor(0, 0);
  lcd.print("Aromatron v1");
  lcd.setCursor(15, 0);
  lcd.write(0);
  lcd.setCursor(0, 1);
  lcd.print("by Leo Keil");
  delay(1000);
  clearflag = true;
}

/*************************************************
++++++++++++[MAIN LOOP]+++++++++++++++++++++++++++
*************************************************/
void loop()
{
  //----------UPDATE TIME----------
  now = rtc.now();
  
  //----------CHECK 1: ALARM---------
  if (isArmed)
  {
    //TODO: Implement Actual check
  }

  //----------CHECK 2: TIME/DATE CHANGE----------
  //check if we're in the main menu
  if (menuState == 0)
  {
    //compare times
    if (now != prevNow)
    {
      updateMainScreen(clearflag, true, true, true);
    }
  }

  //----------CHECK 3: INPUT----------
  //Arming Button
  //check if you're in the main menu
  if (menuState == 0)
  {
    //check if the arming button is pressed
    if (digitalRead(pinArmBtn))
    {
      //show arming dialogue and check if arming/disarming was successful
      if(showArming())
      {
        
      }
    }
  }

  //----------ARCHIVE TIME----------
  prevNow = now;
}

/*************************************************
++++++++++++[METHODS]+++++++++++++++++++++++++++
*************************************************/
//----------checkDial()----------
//Checks if they dial has been turned or not
//0 = no turn, 1 = left turn, 2 = right turn, 3 = button pressed
int checkDial()
{
  int result = 0;
  int rotaryCurState = digitalRead(pinRotIn1);
  //has button been pressed
  if (digitalRead(pinRotBtn))
  {
    //button pressed
    result = 3;
  }
  //has dial turned? 
  else if (rotaryCurState != rotaryLastState)
  {
    //has it turned right?
    if (digitalRead(pinRotIn2) != rotaryCurState)
    {
       //turned right
       result = 2;
    }
    else
    {
      //turned left
      result = 1;
    }
  }
  
  //archive rotary state
  rotaryLastState = rotaryCurState;
  
  return result;
}

//----------updateMainScreen()----------
//updates the main screen fully or partially
void updateMainScreen(bool resetScreen, bool updateTime, bool updateDate, bool updateArmed)
{
  //clear lcd and rebuild basic symbols
  if (resetScreen)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write(6);
    lcd.setCursor(0,1);
    lcd.write(7);

    //reset clearflag
    clearflag = false;
  }
  
  char dateBuffer[12];
  
  //update time
  if (updateTime)
  {
    //fill buffer with time and new syntax and print it
    sprintf(dateBuffer,"%02u:%02u:%02u ",now.hour(),now.minute(),now.second());
    lcd.setCursor(1,0);
    lcd.print(dateBuffer);
  }

  //update date
  if (updateDate)
  {
    //fill buffer with date and new syntax and print it
    sprintf(dateBuffer,"%02u/%02u/%04u ",now.day(),now.month(),now.year());
    lcd.setCursor(1,1);
    lcd.print(dateBuffer);
    
    //print weekday
    lcd.setCursor(12,1);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);

  }

  //update armed status
  if (updateArmed)
  {
    lcd.setCursor(15,0);
    //check if the Aromatron is armed
    if (isArmed)
    {
      //write armed Symbol
      lcd.write(1);
    }
    else
    {
      //write disarmed Symbol
      lcd.write(2);
    }
  }
}

//----------changeRelais()----------
//controls the relais that is in the Aromaboy & the green LED
void changeRelais(bool turnOn)
{
  if (turnOn)
  {
    //turn on Aromaboy and green LED
    digitalWrite(pinRelais, LOW);
    digitalWrite(pinGreenLed, HIGH);
    Serial.println("[INFO] Relais turned on!");
  }
  else
  {
    //turn off Aromaboy and green LED
    digitalWrite(pinRelais, HIGH);
    digitalWrite(pinGreenLed, LOW);
    Serial.println("[INFO] Relais turned off!");
  }
}

//----------toggleArming()----------
//toggles the arming status and red LED
void toggleArming()
{
  //give little alarm beep and blink
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(pinBuzzer, HIGH);
    digitalWrite(pinRedLed, HIGH);
    delay(30);
    digitalWrite(pinBuzzer, LOW);
    digitalWrite(pinRedLed, LOW);
    delay(80);
  }
  
  //disarming
  if (isArmed)
  {
    isArmed = false;
    digitalWrite(pinRedLed, LOW);
    Serial.println("[INFO] Aromatron disarmed!");
  }
  //arming
  else
  {
    isArmed = true;
    digitalWrite(pinRedLed, HIGH);
    Serial.println("[INFO] Aromatron armed!");
  }
}

//----------showArming----------
//Opens the arming dialogue
//returns true if armed, otherwise returns false
bool showArming()
{
  //Setup lcd
  lcd.clear();
  lcd.setCursor(0,0);
  if (isArmed)
  {
    lcd.print("Disarming...");
  }
  else
  {
    lcd.print("Arming...");
  }
  lcd.setCursor(0,1);
  lcd.print("[");
  lcd.setCursor(15,1);
  lcd.print("]");
  
  //goes from 0 to 14
  int progress = 0;
  bool completed = false;
  
  //check if the button is still being held
  while(digitalRead(pinArmBtn) && !completed)
  {
    //check if the arming is complete
    if (progress > 13)
    {
      completed = true;
    }
    else
    {
      lcd.setCursor(progress + 1, 1);
      lcd.write(3);
      progress++;
      delay(150);
    }
  }

  //Show results on completed
  if (completed)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    if (isArmed)
    {
      lcd.print("Disarming");
    }
    else
    {
      lcd.print("Arming");  
    }
    lcd.setCursor(0,1);
    lcd.print("successful!");

    //toggle actual arming state
    toggleArming();

    delay(700);
  }

  //tell main menu to clear display
  clearflag = true;
  return completed;
}

//----------showError----------
//Opens an error dialogue on the lcd, pausing or stopping any other function
void showError(String errorMessage, bool restart)
{
  //turn off relais
  changeRelais(false);
  
  //print general error
  lcd.clear();
  lcd.home();
  lcd.print("Error!");

  //print error message
  lcd.setCursor(0,1);
  lcd.print(errorMessage);

  //print error message to serial
  Serial.println("[ERROR] " + errorMessage);

  //print restart/wait symbol and either delay or loop
  lcd.setCursor(15,0);
  if (restart)
  {
    //print restart symbol
    lcd.print(4);

    //loop forever
    while(1){}
  }
  else
  {
    //print wait symbol
    lcd.print(5);

    //delay for a bit
    delay(5000);
  }
}
