/*
 Name:		TimeMasterDisplay.ino
 Created:	7/6/2019 9:38:49 AM
 Author:	michael
*/


/*HC12 Radio not used*/
//HC12 Radio config
//9600
//FU3
//Ch30
//Powe +20dbM

//Headers goes here
#include <FastLED.h>
#include "Wire.h"

#include  "NewLiquidCrystal_lib/LiquidCrystal_I2C.h"
#include "Waveform.h"
#include <SPI.h>
#include <RH_RF69.h>



// How many leds in your strip?
#define NUM_LEDS 49
#define NUM_STRIPS 4


//Strip 2 for handling mode, might change to 5 to get 2 more ports
#define NUM_LEDS1 3
#define DATA_PIN 11

// Singleton instance of the radio driver
RH_RF69 rf69;



//LED strips arrays
CRGB strip[(NUM_LEDS*NUM_STRIPS)];
CRGB leds[NUM_LEDS1];
CLEDController *modeLeds;

static CRGB myColor = CRGB::Red;
volatile static int digit, digit1, digit2, digit3;
volatile static int lastdigit, lastdigit1, lastdigit2, lastdigit3;
unsigned long oldTimer, startTime1;
boolean receiveComplete = false;		//Flag to get all data

//Globale defines goes here
//Pin difinitions
#define BACKLIGHT_PIN 3
#define startButton 6
#define stopButton 7
#define automatic_pin 3
#define plus_pin 4
#define minus_pin 5
//#define automatic_stop

//LiquidCrystal_I2C lcd(0x27, 20, 4,);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

//SoftwareSerial HC12(3, 4); // Arduino RX, TX  HC12(TX,RX)

// the setup function runs once when you press reset or power the board
//Enums to handle state mashines
enum states {state0,state1,state2,state3,state4};	//We need more stated to handle 2 more ports, but for now it is ok
enum modes {mode0,mode1,mode2,mode3};				//Mode forhandling automatic start and stop
int State = 0;
static long unsigned int timer = 0;
static long unsigned int currentTime = 0;
static long unsigned int waitTime = 0;
static long unsigned int countDown = 0;
const byte numChars = 40;
unsigned int num_of_errors = 0;
unsigned int cntDownTime = 45;
unsigned int maxTime = 999;
unsigned int timeError = 0;
char receivedChars[numChars];
boolean newData = false;
boolean start = false;
boolean stop = false;
int automatic_start = mode1;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeStart = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimePlus = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeMinus = 0;  // the last time the output pin was toggled

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

int buttonStateStart;             // the current reading from the input pin
int lastButtonStateStart = LOW;   // the previous reading from the input pin

int buttonStatePlus;             // the current reading from the input pin
int lastButtonStatePlus = LOW;   // the previous reading from the input pin

int buttonStateMinus;             // the current reading from the input pin
int lastButtonStateMinus = LOW;   // the previous reading from the input pin

//****************************************************************************************************************


void startTime() {
	timer = millis();
	currentTime = 0;
}

//*****************************************************************************************************************
//TIMER UPDATE
void updateDisplayTime() {
	int timeDiff = 0;


	currentTime = millis() - timer;
	
	int mili = currentTime % 100;
	

	timeDiff =  (currentTime/ 1000)-maxTime;
	
	if (timeDiff > 0) {
		timeError = (timeDiff / 4);
		
	}
	

	
	digit = int((currentTime / 100)) % 10;
	digit1 = int((currentTime / 1000)) % 10;
	digit2 = int((currentTime / 10000)) % 10;
	digit3 = int((currentTime / 100000)) % 10;

	int maxDigit1  = int((maxTime)) % 10;
	int maxDigit2  = int((maxTime/10)) % 10;
	int maxDigit3 = int((maxTime/100)) % 10;

	int errorDigit1 = int((timeError)) % 10;
	int errorDigit2 = int((timeError / 10)) % 10;
	

	//if (mili < 10)
	//	lcd.setCursor(13, 3);
	//else
	lcd.setCursor(11, 1);
	lcd.print(errorDigit2, DEC);
	lcd.print(errorDigit1, DEC);
	
	lcd.setCursor(6, 2);
	lcd.print(num_of_errors, DEC);
	
	 lcd.setCursor(17, 2);
	lcd.print(maxDigit3, DEC);
	lcd.print(maxDigit2, DEC);
	lcd.print(maxDigit1, DEC);

	lcd.setCursor(3, 3);
	lcd.print(digit3, DEC);
	lcd.print(digit2, DEC);
	lcd.print(digit1, DEC);
	
	lcd.setCursor(7, 3);
	lcd.print(digit, DEC);
	//lcd.setCursor(13, 3);
	lcd.print(int((currentTime / 10)) % 10, DEC);

	digitWrite(0,digit, 2);
	digitWrite(1, digit1, 1);
	digitWrite(2, digit2, 1);
	digitWrite(3, digit3, 1);
	delay(4);
	FastLED.show();
	delay(4);
}
//*****************************************************************************************************************


//*****************************************************************************************************************
//Inittilize the LED display
void initLed() {
	// Turn the LED on, then pause


	for (int j = 0; j < NUM_LEDS; j++) {
		strip[j] = CRGB::Red;
		//strip[(NUM_LEDS*NUM_STRIPS)+i] = CRGB::Red;
	}
	for (int j = 49; j < 98; j++) {
		strip[j] = CRGB::Red;
		//strip[(NUM_LEDS*NUM_STRIPS)+i] = CRGB::Red;
	}
	for (int j = 98; j < 3 * NUM_LEDS; j++) {
		strip[j] = CRGB::Red;
		//strip[(NUM_LEDS*NUM_STRIPS)+i] = CRGB::Red;
	}
	for (int j = 147; j < 4 * NUM_LEDS; j++) {
		strip[j] = CRGB::Red;
		//strip[(NUM_LEDS*NUM_STRIPS)+i] = CRGB::Red;
	}
	delay(100);
	FastLED.show();
	FastLED.show();
	Serial.print("Led on");
	delay(2000);

	// Now turn the LED off, then pause
	for (int i = 0; i < NUM_LEDS; i++) {
		strip[i] = CRGB::Black;
		strip[i + 49] = CRGB::Black;
		strip[i + (2 * 49)] = CRGB::Black;
		strip[i + (3 * 49)] = CRGB::Black;
	}
	delay(100);
	FastLED.show();
	Serial.println("led off");
	delay(500);


}
//*****************************************************************************************************************

//*****************************************************************************************************************
//Setup for all the Hardware
void setup() {
	FastLED.addLeds<WS2811_PORTA, NUM_STRIPS, GRB>(strip, NUM_LEDS);
	Serial.begin(115200);


	modeLeds= &FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS1);
	leds[0] = CRGB::Red;
	leds[1] = CRGB::Red;
	leds[2] = CRGB::Red;

	modeLeds->showLeds(100);
	
	delay(500);
	if (!rf69.init())
		while (true)
		{

			leds[0] = CRGB::Red;
			FastLED.show();
			Serial.println("init failed");
			rf69.init();
			delay(500);
			leds[0] = CRGB::Black;
			FastLED.show();
			delay(500);
		}

// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
// No encryption
	if (!rf69.setFrequency(868.0))
		Serial.println("setFrequency failed");
	delay(500);
	
	FastLED.setBrightness(100);
	
	initLed();


	analogWriteResolution(12);  // set the analog output resolution to 12 bit (4096 levels)
	pinSetup();
	lcd.begin(20, 4);
	initDisplay();
	delay(2000);
	readyForStart();
	delay(2000);
	State = states::state0;
	automatic_start = mode0;
}
//*****************************************************************************************************************


void checkBtn() {
	static bool readyRead = true;
	/******************************************************************************/
	/*Pin reading*/
	int reading = digitalRead(automatic_pin);
	int startReading = digitalRead(startButton);
	int plusReading = digitalRead(plus_pin);
	int minusReading = digitalRead(minus_pin);
	/******************************************************************************/

	/******************************************************************************/
    //Read millis
	if (reading != lastButtonState) {
		// reset the debouncing timer
		lastDebounceTime = millis();
	}

	if (startReading != lastButtonStateStart) {
		lastDebounceTimeStart = millis();
	}


	if (plusReading != lastButtonStatePlus) {
		lastDebounceTimePlus = millis();
	}
	if (minusReading != lastButtonStateMinus) {
		lastDebounceTimeMinus = millis();
	}
	/******************************************************************************/


	/******************************************************************************/
	//Actions


	if ((millis() - lastDebounceTimePlus) > debounceDelay) {


		if (plusReading != buttonStatePlus) {
			buttonStatePlus = plusReading;

			if (buttonStatePlus == LOW) {
				maxTime += 5;
				if (maxTime > 1000) maxTime = 5;
				if (State == state1)
					updateMaxTime();
				
			}
		}
	}


	if ((millis() - lastDebounceTimeMinus) > debounceDelay) {


		if (minusReading != buttonStateMinus) {
			buttonStateMinus = minusReading;

			if (buttonStateMinus == LOW) {
				maxTime -= 5;
				if (maxTime == 0) maxTime = 999;
				if (State == state1)
					updateMaxTime();
				
			}
		}
	}

	

	if ((millis() - lastDebounceTimeStart) > debounceDelay) {
	

		if (startReading != buttonStateStart) {
			buttonStateStart = startReading;

			if (buttonStateStart == LOW) {
				num_of_errors += 4;
			}
		}
	}


	if ((millis() - lastDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer than the debounce
		// delay, so take it as the actual current state:

		// if the button state has changed:
		if (reading != buttonState) {
			buttonState = reading;
			Serial.println("Button state");
			// only toggle the LED if the new button state is HIGH
			if (buttonState == LOW) {
				
				automatic_start = automatic_start+1;
				if (automatic_start > 2) automatic_start = 0;

				Serial.println(automatic_start);
				
				if(automatic_start==mode0) leds[2]= CRGB::Red;
				if (automatic_start == mode1) leds[2] = CRGB::Green;
				if (automatic_start == mode2) leds[2] = CRGB::Yellow;
				
				
				FastLED.show();
			}
		}


	}
	/******************************************************************************/
	
//Store last pin readings
	lastButtonState = reading;
	lastButtonStateStart = startReading;
	lastButtonStatePlus = plusReading;
	lastButtonStateMinus = minusReading;
}


// the loop function runs over and over again until power down or reset
void loop() {
	readRadio();
	checkBtn();
	switch (State)
	{
	case state0: 
		{ 
		Serial.println("state 0");
		num_of_errors = 0;
		timeError = 0;
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
		if (automatic_start == mode1) {
			if (millis() - countDown > 1000) {
				cntDown++;
				Serial.println(cntDown);
				displayCountDown(cntDown);
				if (cntDown > cntDownTime)
				{
					cntDown = 0;
					Serial.println("Timeout on start");
					State = states::state2;
					start = false;
				}
				countDown = millis();
			}
		}
		break;
		}
	case state2: displayTime(); startTime(); State = states::state3; tone(2, 800); delay(150);
		break;
	case state3: {
		updateDisplayTime();
		if ((digitalRead(stopButton) == LOW) || (stop == true)) {
			Serial.println("Got a stop signal");
			State = states::state4;
			stop = false;
			tone(4, 800); 
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

			if ((automatic_start == mode1) || (automatic_start == mode2)) {

				if (millis() - waitTime > 1000) {
					cnt++;
					Serial.println(cnt);
					
					if ((cnt == cntDownTime)||(automatic_start==mode2)) {
						cnt = 0;
						waitTime = 0;
						Serial.println("TimeOut");
						State = states::state0;
						waitforhigh = false;
						stop = false;

					}
					waitTime = millis();
				}
			}


		}
		break;
	}
				 
	}
}

void updateMaxTime()
{
	lcd.setCursor(10, 1);



	int maxDigit1 = int((maxTime)) % 10;
	int maxDigit2 = int((maxTime / 10)) % 10;
	int maxDigit3 = int((maxTime / 100)) % 10;

	lcd.print(maxDigit3, DEC);
	lcd.print(maxDigit2, DEC);
	lcd.print(maxDigit1, DEC);

}

void displayCountDown(int cnt) {
	lcd.setCursor(6, 1);
	lcd.print("Tid :: ");
	lcd.setCursor(13, 1);
	if (cntDownTime - cnt < 10)
		lcd.print("0");
	
		
	lcd.print(cntDownTime -cnt,DEC);
	digitWrite(0, (cntDownTime - cnt) % 10, 1);
	digitWrite(1, ((cntDownTime - cnt)/ 10) % 10, 1);
	digitWrite(2,8, 0);
	digitWrite(3, 8, 0);
	delay(10);
	FastLED.show();
	delay(10);
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
	lcd.print("  Powered By Rahr");
}

void readyForStart() {
	
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("Klar til start");
	lcd.setCursor(0, 1);
	lcd.print("Max Time =");
	lcd.print(maxTime, DEC);
	lcd.setCursor(0, 2);
	lcd.print("--------------------");
	lcd.setCursor(1, 3);
	lcd.print("Venter paa signal.");
}

void displayTime() {
	lcd.clear();
	lcd.setCursor(5, 0);
	lcd.print("Tid korer");
	lcd.setCursor(0, 1);
	lcd.print("Tidsfejl = ");
	lcd.print(timeError, DEC);
	lcd.setCursor(0, 2);
	lcd.print("Fejl = ");
	lcd.setCursor(10, 2);
	lcd.print("Max T= ");
	lcd.print(maxTime,DEC);
	lcd.setCursor(0, 3);
	lcd.print("S1_000:00");
	lcd.setCursor(11, 3);
	lcd.print("S2_000:00");
}

void finalTime()
{
	lcd.setCursor(5, 0);
	lcd.print("Total tid");
	lcd.setCursor(0, 1);
	lcd.print("Tryk stop for ny tid");


	if (int((currentTime / 10000)) < 10) {
		digit = int((currentTime / 10)) % 10;
		digit1 = int((currentTime / 100)) % 10;
		digit2 = int((currentTime / 1000)) % 10;
		digit3 = int((currentTime / 10000)) % 10;


		digitWrite(0, digit, 2);
		digitWrite(1, digit1, 2);
		digitWrite(2, digit2, 1);
		digitWrite(3, digit3, 1);
		delay(10);
		FastLED.show();
	}


}

void pinSetup() {
	Serial.println("Setup PINS");
	pinMode(startButton, INPUT_PULLUP);
	pinMode(stopButton, INPUT_PULLUP);
	pinMode(automatic_pin, INPUT_PULLUP);
	pinMode(plus_pin, INPUT_PULLUP);
	pinMode(minus_pin, INPUT_PULLUP);
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
		else if (strcmp(receivedChars, "STARTALIVE") == 0)
		{
		leds[0]= CRGB::Green;
		FastLED.show();
		}
		else if (strcmp(receivedChars, "STOPALIVE") == 0)
		{
			leds[1] = CRGB::Green;
			FastLED.show();
		}
	}
}


void recvWithStartEndMarkers() {
	static boolean recvInProgress = false;
	static byte ndx = 0;
	char startMarker = '<';
	char endMarker = '>';
	char rc;
	
	
	//Serial.println("Check Radio");
	if (rf69.available())
	{

		uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);
		for (int i = 0; i < numChars; i++)
			receivedChars[i] = '\0';

		if (rf69.recv(buf, &len))
		{
			RH_RF69::printBuffer("request: ", buf, len);
			
			Serial.println((char*)buf);
			Serial.print("RSSI: ");
			Serial.println(rf69.lastRssi(), DEC);
		
			if (buf[0] == '<' && buf[len-2] == '>') {
				Serial.println("Got something for us");
				
				for (int i = 1; i < len-2 ; i++)
					receivedChars[i-1] = buf[i];

				newData = true;
				Serial.println(receivedChars);
			}

		}
		else
		{
			Serial.println("recv failed");
			newData = false;
		}

		
		
	}
}

void tone(int number, unsigned long int holdTime)
{
	for (int j = 0; j < number; j++) {
		for (int i = 1; i < maxSamplesNum - 1; i++) {
			analogWrite(DAC0, waveformsTable[i]);  // write the selected waveform on DAC0

			delayMicroseconds(holdTime);  // Hold the sample value for the sample tim
		}
		analogWrite(DAC0, 0);  // write the selected waveform on DAC0

	}



}


////////////////////////////////////////////////////////////////////////////////
void digitWrite(int digit, int val, int col) {

	//use this to light up a digit
	//'digit' is which one (right to left, 0 indexed)
	//'val' is the value to set on the digit
	//'col' is the color to use, R,G,B or W
	//example: 
	//        digitWrite(0, 4, 2); 
	//this would set the digit
	//on the far right to a "4" in green.

  /*
  // Letters are the standard naming, numbers are based upon the wiring sequence

			A 2
	   ----------
	  |          |
	  |          |
  F 1 |          | B 3
	  |          |
	  |     G 7  |
	   ----------
	  |          |
	  |          |
  E 6 |          | C 4
	  |          |
	  |     D 5  |
	   ----------    dp 8

  */
  //these are the numeric character definitions, 
  //if last argument is a 0, the segment is off



	if (val == 0) { // "0"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, col);
		segLight(digit, 2, col);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, col);
		segLight(digit, 6, col);
		segLight(digit, 7, 0);
		segLight(digit, 8, col);
	}
	if (val == 1) { // "1"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, 0);
		segLight(digit, 2, 0);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, 0);
		segLight(digit, 6, 0);
		segLight(digit, 7, 0);
		segLight(digit, 8, col);
	}
	if (val == 2) { // "2"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, 0);
		segLight(digit, 2, col);
		segLight(digit, 3, col);
		segLight(digit, 4, 0);
		segLight(digit, 5, col);
		segLight(digit, 6, col);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
	if (val == 3) { // "3"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, 0);
		segLight(digit, 2, col);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, col);
		segLight(digit, 6, 0);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
	if (val == 4) { // "4"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, col);
		segLight(digit, 2, 0);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, 0);
		segLight(digit, 6, 0);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
	if (val == 5) { // "5"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, col);
		segLight(digit, 2, col);
		segLight(digit, 3, 0);
		segLight(digit, 4, col);
		segLight(digit, 5, col);
		segLight(digit, 6, 0);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
	if (val == 6) { // "6"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, col);
		segLight(digit, 2, col);
		segLight(digit, 3, 0);
		segLight(digit, 4, col);
		segLight(digit, 5, col);
		segLight(digit, 6, col);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
	if (val == 7) { // "7"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, 0);
		segLight(digit, 2, col);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, 0);
		segLight(digit, 6, 0);
		segLight(digit, 7, 0);
		segLight(digit, 8, col);
	}
	if (val == 8) { // "8"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, col);
		segLight(digit, 2, col);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, col);
		segLight(digit, 6, col);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
	if (val == 9) { // "9"
	  //segments A,B,C,D,E,F
		segLight(digit, 1, col);
		segLight(digit, 2, col);
		segLight(digit, 3, col);
		segLight(digit, 4, col);
		segLight(digit, 5, col);
		segLight(digit, 6, 0);
		segLight(digit, 7, col);
		segLight(digit, 8, col);
	}
}
//END void digitWrite(int digit, int val, int col)
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void segLight(char digit, int seg, int col) {

	//'digit' picks which neopixel strip
	//'seg' calls a segment
	//'col' is color
	/*
	//always blank the LEDs first
	for(int i = 0; i<56; i++){
	  strip[0].setPixelColor(i, 0, 0, 0);
	}
  */
	int color[3];

	//color sets
	if (col == 0) { //off
		color[0] = { 0 };
		color[1] = { 0 };
		color[2] = { 0 };
	}
	if (col == 1) { //red
		color[0] = { 255 };
		color[1] = { 0 };
		color[2] = { 0 };
	}
	if (col == 2) { //green
		color[0] = { 255 };
		color[1] = { 0 };
		color[2] = { 127 };
	}
	if (col == 3) { //blue
		color[0] = { 0 };
		color[1] = { 0 };
		color[2] = { 255 };
	}
	if (col == 4) { //white -- careful with this one, 3x power consumption
		color[0] = { 255 };
		color[1] = { 255 };
		color[2] = { 255 };
	}
	if (col == 5) { //yellow
		color[0] = { 200 };
		color[1] = { 120 };
		color[2] = { 0 };
	}



	//sets are 0-7, 
	//8-15, 16-23, 24-31, 32-39, 40-47, 48-55 

	//seg F
	if (seg == 1) {
		//light first 8
		for (int i = 0; i < 7; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);
		}
		//strip(digit).show();
		//FastLED.show();
	}
	//seg A
	if (seg == 2) {
		//light second 8
		for (int i = 7; i < 14; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);;
		}
		//strip(digit).show();
		//FastLED.show();
	}
	//seg B
	if (seg == 3) {
		for (int i = 14; i < 21; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);
		}
		//strip(digit).show();
	}
	//seg C
	if (seg == 4) {
		for (int i = 21; i < 28; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);
		}
		//strip.show();
		//FastLED.show();
	}
	//seg D
	if (seg == 5) {
		for (int i = 28; i < 35; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);
		}
		//strip(digit).show();
	}
	//seg E
	if (seg == 6) {
		for (int i = 35; i < 42; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);
		}
		//strip(digit).show();
		//FastLED.show();
	}
	//seg G
	if (seg == 7) {
		for (int i = 42; i < 49; i++) {
			strip[i + (digit * 49)] = CRGB(color[0], color[1], color[2]);
		}
		//strip(digit).show();
		//FastLED.show();
	}
	//seg dp

}
//END void segLight(char digit, int seg, int col)
////////////////////////////////////////////////////////////////////////////////


