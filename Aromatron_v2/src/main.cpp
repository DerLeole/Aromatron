/******************************************************************
 Project     :  Aromatron v2.0
 Author      :  Leo Keil
 Description :  Melitta Aromaboy automation to make coffee every morning automatically.
 ------------------------------------------------------------------
 [Hardware]
 Inteded for the following hardware, but probably usable for other setups, if you adjust the pins.
 -Arduino Nano
 -DS3231 I2C RTC
 -16x2 I2C LCD
 -Rotary Encoder with button
 -Button
 -2 LEDs (green & red)
 -Piezo Buzzer
 -Relais (bridging the 2 main AC cables inside the coffee machine in parallel to the machine's)
 -Melitta Aromatron (it's cheap, makes good coffee, is easy to take apart, and has space for the relais).

 [Config]
 I provided a couple of options you can set before compiling to change some things before compiling,
 so you dont have to look at my actual code (yeah, I know it's faults, I just don't have the time to fix them all).
 You can find them in the "DEFINTIONS" section right under the libraries.

 EnableEasyTimeSetup: uses the computer time/date on compilation to initialize the RTC every restart.
 EepromOffset: This setting allows you to save persistent data to higher numbered registers, instead of the usual first few.

 [Libraries]
 RTCLib:
 LiquidCrystal_I2C:
 Wire:
 EEPROMex (includes EEPROMVar.h):
 MD_REncoder (seriously the best RotaryEncoder Input library):
******************************************************************/

/*************************************************
++++++++++++[DEFINITIONS]+++++++++++++++++++++++++
*************************************************/
//----------[Libraries]----------
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROMVar.h>
#include <EEPROMex.h>
#include <MD_REncoder.h>
#include "customCharacters.h"
#include <Arduino.h>
#include "main.h"

//----------[Config]----------
static bool EnableEasyTimeSetup = true;
static int EepromOffset = 0;

//----------[Pins]----------
#define pinRotIn1 2
#define pinRotIn2 3
#define pinRotBtn 4
#define pinBuzzer 5
#define pinArmBtn 6
#define pinRedLed 7
#define pinGreenLed 8
#define pinRelais 12

//----------[Display]----------
#define numberOfInterruptables 13

//----------[LCD]----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

//----------[RTC]----------
RTC_DS3231 rtc;

//----------[RotaryEncoder]----------
MD_REncoder RotaryEncoder = MD_REncoder(3, 2);


/*************************************************
++++++++++++[VARIABLES]+++++++++++++++++++++++++++
*************************************************/
//General
bool isArmed;
bool alarmIsAudible;

//Display

//Time
DateTime now;
int brewTime;
int warmTime;
//starts with monday
int alarmTimes[7];
//(Starts with Sunday)
//const static char daysOfTheWeekFull[7][9] PROGMEM = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

//Rotary Encoder
bool rotaryButtonLocked = false;
int rotarySpeed;

//EEPROM

/*************************************************
++++++++++++[SETUP]+++++++++++++++++++++++++++++++
*************************************************/
void setup() 
{
	//Initialize Debug Serial Connection
	Serial.begin(9600);

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

	//Initialize Wire I2C library
	Wire.begin();

	//Initialize LCD
	lcd.init();
	lcd.noAutoscroll();
	lcd.createChar(0, charCoffeeHeart);
	lcd.createChar(1, charCoffeeArmed);
	lcd.createChar(2, charCoffeeDisarmed);
	lcd.createChar(3, charBean);
	lcd.createChar(4, charRestart);
	lcd.createChar(5, charReturn);
	lcd.createChar(6, charClock);
	lcd.createChar(7, charCalendar);
	lcd.backlight();
	//LCD: print bootscreen
	lcd.setCursor(0, 0);
	lcd.print("Aromatron v2.0");
	lcd.setCursor(15, 0);
	lcd.write(0);
	lcd.setCursor(0, 1);
	lcd.print("by Leo Keil");

	//Initialize Rotary Encoder & library
	RotaryEncoder.begin();

	//Initialize RTC
	rtc.begin();
	rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	//check if rtc lost power
	if (rtc.lostPower())
	{
		//throw error when rtc lost power
		//showError("RTC LOST POWER", false);

		//open time input dialogue
		//showResetTime();
	}

	//Restore Alarms & Co from EEPROM
	//loadFromEeprom();

	//get current time from rtc
	now = rtc.now();

	//Remove the delay if you dont wanna see my "bootscreen"
	delay(500);
	lcd.clear();
}

/*************************************************
++++++++++++[MAIN LOOP]+++++++++++++++++++++++++++
*************************************************/
void loop() 
{
	//check if time has changed
	if (checkTime())
	{
		
	}
}

/*************************************************
++++++++++++[METHODS]+++++++++++++++++++++++++++++
*************************************************/

//----------changeRelais()----------
//controls the relais that is in the Aromaboy & the green LED indicating relay status
void changeRelais(bool turnOn)
{
	if (turnOn)
	{
		//turn on Aromaboy relais and green LED
		digitalWrite(pinRelais, LOW);
		digitalWrite(pinGreenLed, HIGH);
	}
	else
	{
		//turn off Aromaboy relais and green LED
		digitalWrite(pinRelais, HIGH);
		digitalWrite(pinGreenLed, LOW);
	}
}

//----------checkTime()----------
//Updates the "now" variable and returns true if the time has changed since last check and false if not.
bool checkTime()
{
	//get current time from RTC
	DateTime newNow = rtc.now();

	//check if new time is after previous time
	if (newNow > now)
	{
		//save newNow
		now = newNow;

		return true;
	}
	else
	{
		return false;
	}
	
}