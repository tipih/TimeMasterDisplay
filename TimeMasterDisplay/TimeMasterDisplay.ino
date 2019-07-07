/*
 Name:		TimeMasterDisplay.ino
 Created:	7/6/2019 9:38:49 AM
 Author:	michael
*/

//HC12 Radio config
//9600
//FU3
//Ch30
//Powe +20dbM

//Headers goes here
#include "Wire.h"
#include  "NewLiquidCrystal_lib/LiquidCrystal_I2C.h"
#include <SoftwareSerial.h>



//Globale defines goes here
#define BACKLIGHT_PIN 3
#define startButton 6
#define stopButton 7
//#define automatic_stop
#define automatic_start
//LiquidCrystal_I2C lcd(0x27, 20, 4,);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);
SoftwareSerial HC12(3, 4); // Arduino RX, TX  HC12(TX,RX)
// the setup function runs once when you press reset or power the board
enum states {state0,state1,state2,state3,state4};
int State = 0;
static long unsigned int time = 0;
static long unsigned int currentTime = 0;
static long unsigned int waitTime = 0;
static long unsigned int countDown = 0;
const byte numChars = 40;
char receivedChars[numChars];
boolean newData = false;
boolean start = false;
boolean stop = false;

//****************************************************************************************************************


void startTime() {
	time = millis();
	currentTime = 0;
}

void updateDisplayTime() {
	long day = 86400000; // 86400000 milliseconds in a day
	long hour = 3600000; // 3600000 milliseconds in an hour
	long minute = 60000; // 60000 milliseconds in a minute
	long second = 1000; // 1000 milliseconds in a second

	
	currentTime = millis() - time;
	
	int minutes = ((currentTime % day) % hour) / minute;         //and so on...
	int seconds = (((currentTime % day) % hour) % minute) / second;
	int mili = currentTime % 100;
	//Serial.println(mili);

	if (mili < 10)
		lcd.setCursor(13, 3);
	else
		lcd.setCursor(12, 3);
		
	lcd.print(mili, DEC);
	
	if (seconds<10)
	 lcd.setCursor(10, 3);
	else
	 lcd.setCursor(9, 3);
	
	lcd.print(seconds, DEC);
	
	if (minutes < 10)
		lcd.setCursor(7, 3);
	else
		lcd.setCursor(6, 3);
	
	lcd.print(minutes, DEC);
	
}



void setup() {
	Serial.begin(115200);
	HC12.begin(9600);
	pinSetup();
	lcd.begin(20, 4);
	initDisplay();
	delay(2000);
	readyForStart();
	delay(2000);
	State = states::state0;
}

// the loop function runs over and over again until power down or reset
void loop() {
	readRadio();
	switch (State)
	{
	case state0: 
		{ 
		Serial.println("state 0");
		readyForStart();
		State = states::state1;
		countDown = millis();
		break; 
		}
	case state1: {
		
		static int cntDown = 0;
		if ((digitalRead(startButton) == LOW) || (start==true)) {
		Serial.println("Got a start signal");
		State = states::state2;
		start = false;
		cntDown = 0;
		}
#ifdef automatic_start
		if (millis() - countDown > 1000) {
			cntDown++;
			Serial.println(cntDown);
			displayCountDown(cntDown);
			if (cntDown > 29)
			{
				cntDown = 0;
				Serial.println("Timeout on start");
				State = states::state2;
				start = false;
			}
			countDown = millis();
		}
#endif // automatic_start
		break;
		}
	case state2: displayTime(); startTime(); State = states::state3; tone(9, 50); delay(150);
		noTone(9); break;
	case state3: {
		updateDisplayTime();
		if ((digitalRead(stopButton) == LOW) || (stop == true)) {
			Serial.println("Got a stop signal");
			State = states::state4;
			stop = false;
			tone(9, 50); 
			delay(150);
			noTone(9);
			tone(9, 50);
			delay(150);
			noTone(9);
			waitTime = millis();
		}


		
		break;
	}
	case state4: {
		finalTime();
		static bool waitforhigh = false;
		static int cnt = 0;
	
		if (digitalRead(stopButton) == HIGH)
			waitforhigh = true;

	    if ((waitforhigh==true)||(stop==true))
		{
			if ((digitalRead(stopButton) == LOW) || (stop == true)) {
				Serial.println("Got a stop signal");
				State = states::state0;
				waitforhigh = false;
				stop = false;

			}
#ifdef automatic_stop



			if (millis() - waitTime > 1000) {
				cnt++;
				Serial.println(cnt);
				if (cnt == 30) {
					cnt = 0;
					waitTime = 0;
					Serial.println("TimeOut");
					State = states::state0;
					waitforhigh = false;
					stop = false;

				}
				waitTime = millis();
			}
#endif // automatic_stop

		}
		break;
	}
				 
	}
}

void displayCountDown(int cnt) {
	lcd.setCursor(6, 1);
	lcd.print("Tid :: ");
	lcd.setCursor(13, 1);
	if (30 - cnt < 10)
		lcd.print("0");
	
		
	lcd.print(30-cnt,DEC);
}


void initDisplay() {

	lcd.begin(20, 4);                      // initialize the lcd 
		// Print a message to the LCD.
	lcd.backlight();
	lcd.setCursor(4, 0);
	lcd.print("Velkommen til ");
	lcd.setCursor(4, 1);
	lcd.print("Timetracker !");
	lcd.setCursor(1, 2);
	lcd.print("Roskilde rideklub.");
	lcd.setCursor(1, 3);
	lcd.print("Power By DJ / Rahr");
}

void readyForStart() {
	
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("Klar til start");
	lcd.setCursor(0, 2);
	lcd.print("--------------------");
	lcd.setCursor(1, 3);
	lcd.print("Venter paa signal.");
}

void displayTime() {
	lcd.clear();
	lcd.setCursor(5, 0);
	lcd.print("Tid korer");
	lcd.setCursor(2, 1);
	lcd.print("Venter paa stop");
	lcd.setCursor(0, 2);
	lcd.print("--------------------");
	lcd.setCursor(6, 3);
	lcd.print("00:00:00");
}

void finalTime()
{
	lcd.setCursor(5, 0);
	lcd.print("Total tid");
	lcd.setCursor(0, 1);
	lcd.print("Tryk stop for ny tid");
}

void pinSetup() {
	Serial.println("Setup PINS");
	pinMode(startButton, INPUT_PULLUP);
	pinMode(stopButton, INPUT_PULLUP);
	pinMode(9, OUTPUT);
	
}

void readRadio() {
	recvWithStartEndMarkers();

	if (newData == true) {
		newData = false;
		if (strcmp(receivedChars, "START") == 0)
		{
			if (State == states::state1) {
				start = true;
				Serial.print("Got a start ");
				Serial.println(start);
			}
			else {
				//Serial.print("Ignore start signal");
				
			}
		}
		
		else if (strcmp(receivedChars, "STOP") == 0)
		{
			if (State == states::state3) {
				stop = true;
				Serial.print("Got a stop ");
				Serial.println(stop);
			}
			else {
				//Serial.print("Ignore stop signal");
			}
		}
	}
}


void recvWithStartEndMarkers() {
	static boolean recvInProgress = false;
	static byte ndx = 0;
	char startMarker = '<';
	char endMarker = '>';
	char rc;
	
	while (HC12.available() > 0 && newData == false) {
		
		rc = HC12.read();
		if (recvInProgress == true) {
			if (rc != endMarker) {
				receivedChars[ndx] = rc;
				ndx++;
				if (ndx >= numChars) {
					ndx = numChars - 1;
				}
			}
			else {
				receivedChars[ndx] = '\0';
				recvInProgress = false;
				ndx = 0;
				newData = true;
			}
		}
		else if (rc == startMarker) {
			recvInProgress = true;
		}
	}
}
