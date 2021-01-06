/******************************************************************
 Project     :  Aromatron v1
 Author      :  Leo Keil
 Description :  Melitta Aromaboy automation to make coffee every morning automatically.
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
bool alarmIsAudible;
int menuState;

//Display
bool clearflag;
bool editMode;

//Time
DateTime now;
DateTime prevNow;
DateTime alarmTimes[7];
int alarmEditTime;
char daysOfTheWeek[7][2] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
char daysOfTheWeekFull[7][2] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

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
	lcd.createChar(5, charReturn);
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
	rotaryLastState = 0;

	Serial.println("[INFO] Setup completed.");

	//----------POST SETUP----------
	//give little alarm beep
	for (int i = 0; i < 5; i++)
	{
		digitalWrite(pinBuzzer, HIGH);
		digitalWrite(pinRedLed, HIGH);
		delay(30);
		digitalWrite(pinBuzzer, LOW);
		digitalWrite(pinRedLed, LOW);
		delay(80);
	}
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
			updateMenu();
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
			if (showArming())
			{
				Serial.println("[INFO] Armed successfully!");
			}
			else
			{
				Serial.println("[INFO] Disarmed successfully!");

			}
		}
	}

	//Rotary Encoder
	int dialState = checkDial();
	//Check in which menu we currently are
	switch (menuState)
	{
	//Main Menu
	case 0:
		//checks what direction the dial was turned
		switch (dialState)
		{
		//turn left
		case 1:
			//turn off backlight
			//TODO: turn off lighting
			break;
		//turn right
		case 2:
			//turn on backlight
			break;
		//turn right
		case 3:
			// do something
			break;
		default:
			//do nothing
			break;
		}
		break;

	//Settings Menu 1
	case 1:
		// do something
		break;
	default:
		// do something
	}

	//----------ARCHIVE TIME----------
	prevNow = now;
}

/*************************************************
++++++++++++[METHODS]+++++++++++++++++++++++++++
*************************************************/
//----------updateMenu()----------
//updates all interruptable menus according to imputs and global states
void updateMenu()
{
	//checks if it should clear the screen
	//screen is the rebuilt inside the switch
	if (clearflag)
	{
		lcd.clear();
	}

	//check which menu state is active
	switch (menuState)
	{
	//Main Menu
	case 0:
		//rebuild basic symbols after display clear
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.write(6);
			lcd.setCursor(0, 1);
			lcd.write(7);
		}

		char dateBuffer[12];
		//fill buffer with Time and new syntax and print it
		sprintf(dateBuffer, "%02u:%02u:%02u ", now.hour(), now.minute(), now.second());
		lcd.setCursor(1, 0);
		lcd.print(dateBuffer);
		//fill buffer with Date and new syntax and print it
		sprintf(dateBuffer, "%02u/%02u/%04u ", now.day(), now.month(), now.year());
		lcd.setCursor(1, 1);
		lcd.print(dateBuffer);
		//print Weekday
		lcd.setCursor(12, 1);
		lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
		//check if the Aromatron is Armed
		lcd.setCursor(15, 0);
		if (isArmed)
		{
			//write Armed symbol
			lcd.write(1);
		}
		else
		{
			//write Disarmed symbol
			lcd.write(2);
		}
		break;

	//Settings Menu Monday - Sunday
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		//convert menu state to day of the week int
		int dayInt = menuState % 7;

		//check if the basic symbols have to be rebuilt
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print(daysOfTheWeekFull[dayInt]);
			lcd.setCursor(0, 1);
			lcd.write(6);
		}

		//check if edit brackets should be applied
		//O _ _ _ _ > 2 4 : 0 0 < _ _ _ _
		//0 1 2 3 4 5 6 7 8 9 A B C D E F
		lcd.setCursor(5, 1);
		if (editMode)
		{
			lcd.print(">  :  <");
		}
		else
		{
			lcd.print("   :   ");
		}

		//convert saved int to hours and minutes
		int currentEditHours = alarmEditTime / 60;
		int currentEditMinutes = alarmEditTime % 60;
		//print time
		lcd.setCursor(6, 1);
		lcd.print(currentEditHours);
		lcd.setCursor(9, 1);
		lcd.print(currentEditMinutes);
		break;

	//Settings Menu Brewtime
	case 8:
		//check if the basic symbols have to be rebuilt
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Brewtime:");
			lcd.setCursor(0, 1);
			lcd.write(6);
		}

		//check if edit brackets should be applied
		//O _ _ _ > 2 4 m 0 0 s < _ _ _ _
		//0 1 2 3 4 5 6 7 8 9 A B C D E F
		lcd.setCursor(4, 1);
		if (editMode)
		{
			lcd.print(">  m  s<");
		}
		else
		{
			lcd.print("   m  s ");
		}

		//convert saved int to minutes and seconds
		int currentEditMinutes = alarmEditTime / 60;
		int currentEditSeconds = alarmEditTime % 60;
		//print time
		lcd.setCursor(5, 1);
		lcd.print(currentEditMinutes);
		lcd.setCursor(8, 1);
		lcd.print(currentEditSeconds);
		break;

	//Settings Menu Warmtime
	case 9:
		//check if the basic symbols have to be rebuilt
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Warmtime:");
			lcd.setCursor(0, 1);
			lcd.write(6);
		}

		//check if edit brackets should be applied
		//O _ _ _ _ > 4 h 0 0 m < _ _ _ _
		//0 1 2 3 4 5 6 7 8 9 A B C D E F
		lcd.setCursor(5, 1);
		if (editMode)
		{
			lcd.print("> h  m<");
		}
		else
		{
			lcd.print("  h  m ");
		}

		//convert saved int to minutes and seconds
		int currentEditHours = alarmEditTime / 60;
		int currentEditMinutes = alarmEditTime % 60;
		//print time
		lcd.setCursor(6, 1);
		lcd.print(currentEditHours);
		lcd.setCursor(8, 1);
		lcd.print(currentEditMinutes);
		break;

	//Settings Menu Audible Alarm
	case 10:
		//check if basic symbols have to be rebuilt
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Audible Alarm?");
		}

		//check if it should display enabled or disabled
		//_ _ _ [ E N A B L E D ] _ _ _ _
		//0 1 2 3 4 5 6 7 8 9 A B C D E F
		lcd.setCursor(3, 1);
		if (alarmIsAudible)
		{
			lcd.print("[ENABLED]");
		}
		else
		{
			lcd.print("[DISABLED]");
		}
		break;

	//Settings Menu Reset Time
	case 11:
		//check if basic symbols have to be rebuilt
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Reset Time/Date?");
		}

		//check what confirmation stage it is in
		//[ R E A L L Y   R E S E T ? ] _
		//0 1 2 3 4 5 6 7 8 9 A B C D E F
		lcd.setCursor(3, 1);
		if (editMode)
		{
			lcd.print("[REALLY RESET?]");
		}
		else
		{
			lcd.print("[CLICK TO RESET]");
		}
		break;

	//Settings Return to Main Menu
	case 12:
		//check if basic symbols have to be rebuilt
		if (clearflag)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Back to");
			lcd.setCursor(15, 0);
			lcd.write(5);
			lcd.setCursor(0, 1);
			lcd.print("Main Menu?");
		}
		break;

	default:
		//Throw an error
		showError("InvalidMenuState", false);
	}

	//reset the clearflag
	clearflag = false;
}

//----------showArming()----------
//Opens the arming dialogue
//returns true if armed, otherwise returns false
bool showArming()
{
	//change menuState
	menuState = -2;

	//Setup lcd
	lcd.clear();
	lcd.setCursor(0, 0);
	if (isArmed)
	{
		lcd.print("Disarming...");
	}
	else
	{
		lcd.print("Arming...");
	}
	lcd.setCursor(0, 1);
	lcd.print("[");
	lcd.setCursor(15, 1);
	lcd.print("]");

	//goes from 0 to 14
	int progress = 0;
	bool completed = false;

	//check if the button is still being held
	while (digitalRead(pinArmBtn) && !completed)
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
		lcd.setCursor(0, 0);
		if (isArmed)
		{
			lcd.print("Disarming");
		}
		else
		{
			lcd.print("Arming");
		}
		lcd.setCursor(0, 1);
		lcd.print("successful!");

		//toggle actual arming state
		toggleArming();

		delay(700);
	}

	//tell main menu to clear display
	menuState = 0;
	clearflag = true;
	return completed;
}

//----------showError()----------
//Opens an error dialogue on the lcd, pausing or stopping any other function
void showError(String errorMessage, bool restart)
{
	//change menuState
	menuState = -1;

	//turn off relais
	changeRelais(false);

	//print general error
	lcd.clear();
	lcd.home();
	lcd.print("Error!");

	//print error message
	lcd.setCursor(0, 1);
	lcd.print(errorMessage);

	//print error message to serial
	Serial.println("[ERROR] " + errorMessage);

	//print restart/wait symbol and either loop forever or until button is pressed
	lcd.setCursor(15, 0);
	if (restart)
	{
		//print restart symbol
		lcd.print(4);

		//loop forever
		while (1) {}
	}
	else
	{
		//print wait symbol
		lcd.print(5);

		//loop until dial is moved or pressed
		while (checkDial() == 0) {}
		//tell main menu to clear display
		menuState = 0;
		clearflag = true;
	}
}

//----------showResetTime()----------
//Opens a dialogue that lets you reset the time of the rtc
void showResetTime()
{
	//change menuState
	menuState = -4;

	//variables
	int dateArray[3];
	int hour;
	int minute;
	int second;
	DateTime newTime;
	int timeResetStage = 0;

	//o _ _ 2 5 / 0 4 / 2 0 2 1 _ _ _
	//0 1 2 3 4 5 6 7 8 9 A B C D E F
	//clear lcd and print date input dialogue
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Set Date:");
	lcd.setCursor(0, 1);
	lcd.write(7);
	lcd.setCursor(5, 1);
	lcd.print(" /  / ");

	//date reset loop
	while (timeResetStage < 3)
	{
		int tempDate;
		//devide what to do according to dial state
		switch (checkDial())
		{
		//turn left
		case 1:
			// do something
			break;
		//turn right
		case 2:
			// do something
			break;
		//button pressed
 		case 3:
			// do something
			break;
		default:
			break;
		}
	}

	//tell main menu to clear display
	menuState = 0;
	clearflag = true;
}

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
	//change menuState
	menuState = -2;

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

	//change menuState
	menuState = 0;
}