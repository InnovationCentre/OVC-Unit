#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

#define forwardButton 2
#define backwardsButton 4
#define rightButton 5
#define leftButton 6
#define pingPin 7

int previousPingState = 0;

//Comminucation outgoing message
long OUT_ReceiverID = 0;
#define OUT_SenderID 8
long OUT_MessageID = 0;
long OUT_Value1 = 0;
long OUT_Value2 = 0;
long OUT_Value3 = 0;
long OUT_Checksum = 0;

//Comminucation incomming message
long IN_ReceiverID = 0;
long IN_SenderID = 0;
long IN_MessageID = 0;
long IN_Value1 = 0;
long IN_Value2 = 0;
long IN_Value3 = 0;
long IN_Checksum = 0;

long IN_Raw_Value;
char Message[8];

RCSwitch sender = RCSwitch();
RCSwitch receiver = RCSwitch();

void setup() {

	Serial.begin(9600);

	pinMode(forwardButton, INPUT);
	pinMode(backwardsButton, INPUT);
	pinMode(rightButton, INPUT);
	pinMode(leftButton, INPUT);
	pinMode(pingPin, INPUT);

	// Transmitter is connected to Arduino Pin #10
	sender.enableTransmit(10);
	receiver.enableReceive(0);

	Serial.println("Program started");
}

void loop() {
	if (receiver.available()) {
		handleMessage();
		receiver.resetAvailable();
	}

	//if (digitalRead(forwardButton) == HIGH) {
	//	createMoveCommand(0);
	//}
	//if (digitalRead(backwardsButton) == HIGH) {
	//	createMoveCommand(1);
	//}
	//if (digitalRead(rightButton) == HIGH) {
	//	createMoveCommand(2);
	//}
	//if (digitalRead(leftButton) == HIGH) {
	//	createMoveCommand(3);
	//}

	int pinValue = digitalRead(pingPin);

	if ((previousPingState != pinValue) && pinValue == HIGH) {
		createPingCommand();
	}

	previousPingState = pinValue;
}

void handleMessage() {
	readMessage();

	if (IN_Checksum == calculateChecksum(IN_ReceiverID, IN_SenderID, IN_MessageID, IN_Value1, IN_Value2, IN_Value3)) { //Message is valid
		Serial.print("Message ");
		Serial.print(Message);
		Serial.println(" is a valid message");

		if (IN_ReceiverID == 8 || IN_ReceiverID == 9) { //Message is send to this robot

			switch (IN_MessageID) { //Command type

			case 8: //Acknowledge

				handleAcknowledge();
			}
		}
		else {
			Serial.println("Message is not for this robot");
		}
	}
	else {
		Serial.println("Message is not valid");
		resetMessage();
	}
}

void handleAcknowledge() {
	switch (IN_Value1) { //Acknowledge type

	case 0: //Acknowledge ping received
		Serial.println("PING ACKNOWLEDGE RECEIVED");
		break;
	}

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

void createMoveCommand(int direction) {
	OUT_ReceiverID = 1;
	OUT_MessageID = 4;
	OUT_Value1 = direction;
	OUT_Value2 = 1;
	OUT_Value3 = 0;
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);

	sendMessage();
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

void handleAcknowledge(int messageId) {
	writeOUT_Values(messageId, IN_Checksum, 0);

	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
	sendMessage();
}

/**
* Reset the incomming message variable to 0
*/
void resetMessage() {
	IN_Raw_Value = 00000000;
	readMessage();
}

/**
* Sets the output values of the outgoing message
*/
void writeOUT_Values(long Value1, long Value2, long Value3) {
	OUT_Value1 = Value1;
	OUT_Value2 = Value2;
	OUT_Value3 = Value3;
}

/**
* Send message with the default OUT_-attributes. After the message is send, the OUT values are reset.
*/
void sendMessage() {
	sendMessage(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3, OUT_Checksum);
}

/**
* Send message. After the message is send, the OUT values are reset.
*/
void sendMessage(long receiverId, long senderId, long messageId, long value1, long value2, long value3, long checksum) {
	long message = calculateMessage(receiverId, senderId, messageId, value1, value2, value3, checksum);
	sender.send(message, 24);

	Serial.print("Send message: ");
	Serial.println(message);

	resetMessage();
}

/**
* Read the incomming message and split it to seperate variables0
*/
void readMessage() {
	IN_Raw_Value = receiver.getReceivedValue();
	String stringmessage = String(IN_Raw_Value);
	stringmessage.toCharArray(Message, 8);

	IN_ReceiverID = Message[0] - '0';
	IN_SenderID = Message[1] - '0';
	IN_MessageID = Message[2] - '0';
	IN_Value1 = Message[3] - '0';
	IN_Value2 = Message[4] - '0';
	IN_Value3 = Message[5] - '0';
	IN_Checksum = Message[6] - '0';
}

