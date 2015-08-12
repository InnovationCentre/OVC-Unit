/**
 * Handles the received message. Checks if the message is valid and is
 * responsible for taking actions based on the received message.
 */
void handleMessage() {
	Serial.print("Received message: ");
	Serial.println(mySwitch.getReceivedValue());

	readMessage();

	if (IN_Checksum == calculateChecksum(IN_ReceiverID, IN_SenderID, IN_MessageID, IN_Value1, IN_Value2, IN_Value3)) { //Message is valid
		Serial.print("Message ");
		Serial.print(Message);
		Serial.println(" is a valid message");

		if (IN_ReceiverID == robotId || IN_ReceiverID == broadcastId) { //Message is send to this robot

			OUT_ReceiverID = IN_SenderID;
			OUT_MessageID = 8;

			switch (IN_MessageID) { //Command type

			case 0: //Ping
				writeOUT_Values(0, IN_Checksum, 0);
				break;
			case 1: //Battery status
			  //Read battery 12 V or 24 V according to variable "IN_Value1".
			  //Read percentage between 0 and 99;
			  //Split as two numbers;
			  //Write to these variables
				writeOUT_Values(5, 5, IN_Checksum);
				OUT_MessageID = 1;
				break;
			case 2: //Go to Home
				writeOUT_Values(2, IN_Value1, IN_Checksum);
				break;
			case 3: //GoTo <location>
				writeOUT_Values(3, IN_Checksum, 0);
				break;
			case 4: //Move <direction> <amount>
				writeOUT_Values(4, IN_Checksum, 0);
				handleMove();
				break;
			}

			OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
		}

		else {
			logMessage("Message is not for this robot");
		}

	}

	else {
		logMessage("Message is not valid");
		resetMessage();
	}
}


/**
 * Handles the 'move <direction> <amount>'-command.
 *
 * <direction>:
 *
 * 0 - Straigt forward
 * 1 - Backwards
 * 2 - Right
 * 3 - Left
 *
 * <amount>: amount in centimeters to drive.
 *
 * distance in cm = 2 x (amount +1)
 */
void handleMove() {
	logMessage("MessageID: Drive");

	switch (IN_Value1) {

	case 0: //drive forward
		logMessage("Value1: Forward");
		drive(DriveSpeed, DriveSpeed);
		break;

	case 1: //drive reverse
		logMessage("Value1: Reverse");
		drive(DriveReverseSpeed, DriveReverseSpeed);
		break;

	case 2: //drive right
		logMessage("Value1: Right");
		drive(DriveReverseSpeed, DriveSpeed);
		break;

	case 3: //drive left
		logMessage("Value1: Left");
		drive(DriveSpeed, DriveReverseSpeed);
		break;
	}
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

void sendMessage() {
	Serial.println(OUT_ReceiverID + OUT_SenderID + OUT_MessageID + OUT_Value1 + OUT_Value2 + OUT_Value3 + OUT_Checksum);
}

/**
 * Read the incomming message and split it to seperate variables0
 */
void readMessage() {

	IN_Raw_Value = mySwitch.getReceivedValue();
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
void writeOUT_Values(int Value1, int Value2, int Value3) {
	OUT_Value1 = Value1;
	OUT_Value2 = Value2;
	OUT_Value3 = Value3;
}

/**
 * Handles the ping command. When a ping message is receiver, an acknowledge is returned.
 */
void handlePing() {
	OUT_ReceiverID = 8;
	OUT_MessageID = 0;
	writeOUT_Values(0, 0, 0);
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
	logMessage("Sending Ping");
	sendMessage();
}

/**
 * Send an 'arrived at location' message
 */
void handleArrived() {
	OUT_ReceiverID = 8;
	OUT_MessageID = 3;
	writeOUT_Values(2, 1, 0);
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
	logMessage("Sending Arrived");
	sendMessage();
}

/**
 * Send an error message
 */
void ErrorStop() {
	OUT_ReceiverID = 8;
	OUT_MessageID = 9;
	writeOUT_Values(5, 7, 8);
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
	logMessage("Sending Error");
	sendMessage();
}
