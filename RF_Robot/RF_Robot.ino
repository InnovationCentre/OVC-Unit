#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <Wire.h>

//Communication
const int robotId = 1;
const int broadcastId = 9;
long IN_Raw_Value;
char Message[8];
RCSwitch mySwitch = RCSwitch();

//Debug enables or disables Serial Monitor messages
boolean debug = true;

//Comminucation incomming message
int IN_ReceiverID = 0;
int IN_SenderID = 0;
int IN_MessageID = 0;
int IN_Value1 = 0;
int IN_Value2 = 0;
int IN_Value3 = 0;
int IN_Checksum = 0;

//Comminucation outgoing message
int OUT_ReceiverID = 0;
#define OUT_SenderID 1
int OUT_MessageID = 0;
int OUT_Value1 = 0;
int OUT_Value2 = 0;
int OUT_Value3 = 0;
int OUT_Checksum = 0;

//Motor shield controller
#define ENL 3     //analog PWM port Left
#define MotorL4 4 //digital port IN1
#define MotorL5 5 //digital port IN2
#define ENR 8     //analog PWM port Right
#define MotorR6 6 //digital port IN3
#define MotorR7 7 //digital port IN4

#define CMD                 (byte)0x10                        // Values of 0 eing sent using write have to be cast as a byte to stop them being misinterperted as NULL
#define MD25ADDRESS         0x58                              // Address of the MD25
const byte SPEED1 = (byte)0x00;                        // Byte to send speed to first motor
const byte SPEED2 = 0x01;                              // Byte to send speed to second motor
#define ENCODERONE          0x02                              // Byte to read motor encoder 1
#define ENCODERTWO          0x06                              // Byte to read motor encoder 2
#define VOLTREAD            0x0A                              // Byte to read battery volts
#define RESETENCODERS       0x20                              // Byte to reset encoders

#define DriveSpeed 250  //standard drive speed
#define DriveReverseSpeed 4  //standard drive speed
#define StopSpeed 128   //standard stop speed (255 full forward, 0 full reverse)
#define TurnConst 30
#define PPT 360 //360 pulses per turn
#define WheelDiameter 100 //100mm diameter
#define WheelWidth 470 //distance between middle of wheels

#define powerLed 3
#define drivingLed 4

void setup() {

	pinMode(drivingLed, OUTPUT);
	pinMode(powerLed, OUTPUT);
	digitalWrite(powerLed, HIGH);

	Wire.begin();
	Serial.begin(9600);                                       // Begin serial
	Serial.println("Start program");
	encodeReset();                                            // Cals a function that resets the encoder values to 0
	delay(50);
	SpeedControlReset();
	delay(50);                                                // Wait for everything to power up
	mySwitch.enableReceive(0);
}

void loop() {
	if (mySwitch.available()) {
		handleMessage();
		mySwitch.resetAvailable();
	}
}

/**
 * Show messages at Serialmonitor when debugging is enabled
 */
void logMessage(String message) {
	if (debug) {
		Serial.println(message);
	}
}

/**
* Blink a LED for a certain amount of time
*
* @param timeToBlink Time in mili-seconds of how the LED should blink
* @param driveSpeed ledPin Pin number of which LED should blink
*/
void blinkForTime(long timeToBlink, int ledPin) {
	logMessage("Start blinking");
	unsigned long startTime = millis();
	long previousMillis = 0;
	boolean ledState = false;

	while (millis() - startTime <= timeToBlink) {

		unsigned long currentMillis = millis();

		if (millis() - previousMillis > 500) {
			previousMillis = currentMillis;

			if (ledState) {
				digitalWrite(ledPin, LOW);
			}
			else {
				digitalWrite(ledPin, HIGH);
			}
			ledState = !ledState;
		}
	}

	logMessage("End blinking");
}