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

//----------[Custom Chars]----------
byte charCoffeeHeart[] = {
  B01010,
  B11111,
  B01110,
  B00100,
  B00000,
  B01111,
  B01111,
  B01110
};

byte charCoffeeArmed[] = {
  B00001,
  B01010,
  B00100,
  B00000,
  B00000,
  B01111,
  B01111,
  B01110
};

byte charCoffeeDisarmed[] = {
  B00000,
  B01010,
  B00100,
  B01010,
  B00000,
  B01111,
  B01111,
  B01110
};

byte charBean[] = {
  B01100,
  B11110,
  B10111,
  B11011,
  B11011,
  B11101,
  B01111,
  B00110
};

byte charRestart[] = {
  B00000,
  B11100,
  B01100,
  B10101,
  B10001,
  B10001,
  B01110,
  B00000
};

byte charWait[] = {
  B11111,
  B10001,
  B01010,
  B00100,
  B00100,
  B01110,
  B11111,
  B11111
};

byte charClock[] = {
  B00000,
  B00000,
  B01110,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000
};

byte charClockEdit[] = {
  B00000,
  B10101,
  B00000,
  B01110,
  B10101,
  B10111,
  B10001,
  B01110
};


/*************************************************
++++++++++++[VARIABLES]+++++++++++++++++++++++++++
*************************************************/
//Rotary Encoder
int rotaryCurState;
int rotaryLastState;



/*************************************************
++++++++++++[SETUP]+++++++++++++++++++++++++++++++
*************************************************/
void setup()
{
  //Initialize Debug Serial Connection
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
  lcd.createChar(7, charClockEdit);
  lcd.backlight();

  //LCD: print bootscreen
  lcd.setCursor(0, 0);
  lcd.print("Aromatron v1");
  lcd.setCursor(15, 0);
  lcd.write(0);
  lcd.setCursor(0, 1);
  lcd.print("by Leo Keil");

  //Initialize rotary encoder logic
  rotaryLastState = digitalRead(pinRotIn1);


  Serial.println("[INFO] Setup completed.");
}

/*************************************************
++++++++++++[MAIN LOOP]+++++++++++++++++++++++++++
*************************************************/
void loop()
{
	// write your main code here, to run repeatedly

}

/*************************************************
++++++++++++[METHODS]+++++++++++++++++++++++++++
*************************************************/

//Checks if they dial has been turned or not
//0 = no turn, 1 = left turn, 2 = right turn
int checkDialTurn()
{
  rotaryCurState = digitalRead(pinRotIn1);
  //has dial turned? 
  if (rotaryCurState != rotaryLastState)
  {
    //has it turned right?
    if (digitalRead(pinRotIn2) != rotaryCurState)
    {
       //turned right
       return 2;
    }
    else
    {
      //turned left
      return 1;
    }
  }

  return 0;
}

//Opens an error dialogue on the lcd, pausing or stopping any other function
//
//void showError(int erro



//Extra
/*
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define outputA 2
#define outputB 3
#define rotBtn 4
#define buzzer 5
#define armBtn 6
#define redLED 7
#define greenLED 8

LiquidCrystal_I2C lcd(0x27,16,2); 
 
int counter = 0;
int aState;
int aLastState;
int prevBtn = 1; 


 
void setup() {
pinMode (outputA,INPUT);
pinMode (outputB,INPUT);
pinMode (rotBtn,INPUT_PULLUP);
pinMode (buzzer, OUTPUT);
pinMode (armBtn, INPUT_PULLUP);
pinMode (redLED, OUTPUT);
pinMode (greenLED, OUTPUT);
pinMode (12, OUTPUT);
 
Serial.begin (9600);
 lcd.init();                      
 // Print a message to the LCD.
 lcd.backlight();
 lcd.print("Debug");
// Reads the initial state of the outputA
aLastState = digitalRead(outputA);
 
}
 
void loop() {
int btn = digitalRead(rotBtn);
if (!btn)
{
  digitalWrite(buzzer, HIGH);
}
else
{
  digitalWrite(buzzer, LOW);
}
int aBtn = digitalRead(armBtn);
if (aBtn)
{
  digitalWrite(12, LOW) ;
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, HIGH);
}
else
{
  digitalWrite(12, HIGH);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
}

aState = digitalRead(outputA); // Reads the "current" state of the outputA
// If the previous and the current state of the outputA are different, that means a Pulse has occured
if (aState != aLastState){
// If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
if (digitalRead(outputB) != aState) {
counter ++;
lcd.clear();
} else {
counter --;
lcd.clear();
}

lcd.setCursor(0, 0);
lcd.print("Position: ");
lcd.setCursor(10, 0);
lcd.print(counter);
 
}
aLastState = aState; // Updates the previous state of the outputA with the current state

 
}
*/
