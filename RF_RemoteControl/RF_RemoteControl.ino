/*
  Example for different sending methods

  http://code.google.com/p/rc-switch/

*/

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

#define forwardButton 2
#define backwardsButton 4
#define rightButton 5
#define leftButton 6

//Comminucation outgoing message
long OUT_ReceiverID = 0;
#define OUT_SenderID 8
long OUT_MessageID = 0;
long OUT_Value1 = 0;
long OUT_Value2 = 0;
long OUT_Value3 = 0;
long OUT_Checksum = 0;

RCSwitch sender = RCSwitch();

void setup() {

	Serial.begin(9600);

	pinMode(forwardButton, INPUT);
	pinMode(backwardsButton, INPUT);
	pinMode(rightButton, INPUT);
	pinMode(leftButton, INPUT);

	// Transmitter is connected to Arduino Pin #10
	sender.enableTransmit(10);

	// Optional set pulse length.
	// mySwitch.setPulseLength(320);

	// Optional set protocol (default is 1, will work for most outlets)
	// mySwitch.setProtocol(2);

	// Optional set number of transmission repetitions.
	// mySwitch.setRepeatTransmit(15);

	Serial.println("Program started");
}

void loop() {

	if (digitalRead(forwardButton) == HIGH) {
		createMoveCommand(0);
	}
	if (digitalRead(backwardsButton) == HIGH) {
		createMoveCommand(1);
	}
	if (digitalRead(rightButton) == HIGH) {
		createMoveCommand(2);
	}
	if (digitalRead(leftButton) == HIGH) {
		createMoveCommand();
	}

	//mySwitch.send(5393, 24);
	//delay(1000);
	//mySwitch.send(5396, 24);
	//delay(1000);
}

void createMoveCommand(int direction) {
	OUT_ReceiverID = 1;
	OUT_MessageID = 4;
	OUT_Value1 = direction;
	OUT_Value2 = 1;
	OUT_Value3 = 0;
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);

	sendMessage();
}


void createPingCommand() {
	OUT_ReceiverID = 1;
	OUT_MessageID = 0;
	OUT_Value1 = 0;
	OUT_Value2 = 0;
	OUT_Value3 = 0;
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);

	sendMessage();
}

void sendMessage() {
	Serial.print("Receiver: ");
	Serial.println(OUT_ReceiverID);

	Serial.print("Sender: ");
	Serial.println(OUT_SenderID);

	Serial.print("Message ID: ");
	Serial.println(OUT_MessageID);

	Serial.print("Value1: ");
	Serial.println(OUT_Value1);

	Serial.print("Value2: ");
	Serial.println(OUT_Value2);

	Serial.print("Value3: ");
	Serial.println(OUT_Value3);

	Serial.print("Checksum: ");
	Serial.println(OUT_Checksum);

	long message = calculateMessage(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3, OUT_Checksum);
	sender.send(message, 24);

	Serial.print("Send: ");
	Serial.println(message);

	OUT_ReceiverID = 0;
	OUT_MessageID = 0;
	OUT_Value1 = 0;
	OUT_Value2 = 0;
	OUT_Value3 = 0;
	OUT_Checksum = 0;
}

long calculateMessage(long receiverId, long senderId, long messageId, long value1, long value2, long value3, long checksum) {
	return (receiverId * 1000000)
		+ (senderId * 100000)
		+ (messageId * 10000)
		+ (value1 * 1000)
		+ (value2 * 100)
		+ (value3 * 10)
		+ (checksum);
}

/**
* Calculates the checksum for the given parameters. The checksum is equal
* to the modulo of 8 of the total sum of all parameters.
*
* checksum = (sum(parameters) % 8)
*/
int calculateChecksum(int ReceiverID, int SenderID, int MessageID, int Value1, int Value2, int Value3) {
	int sum = (ReceiverID + SenderID + MessageID + Value1 + Value2 + Value3);
	int modulo = (sum % 8);
	return modulo;
}