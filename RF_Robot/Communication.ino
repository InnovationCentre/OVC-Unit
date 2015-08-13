/**
 * Handles the received message. Checks if the message is valid and is
 * responsible for taking actions based on the received message.
 */
void handleMessage() {
	Serial.print("Received message: ");
	Serial.println(receiver.getReceivedValue());

	readMessage();

	if (IN_Checksum == calculateChecksum(IN_ReceiverID, IN_SenderID, IN_MessageID, IN_Value1, IN_Value2, IN_Value3)) { //Message is valid
		Serial.print("Message ");
		Serial.print(Message);
		Serial.println(" is a valid message");

		if (IN_ReceiverID == robotId || IN_ReceiverID == broadcastId) { //Message is send to this robot
			handleAcknowledge();
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
* Handles all commands which require an acknowledge-message.
*
* <direction>:
*
* 0 - Ping
* 1 - Send battery status
* 2 - Go to home
* 3 - GoTo <location>
*/
void handleAcknowledge() {
	OUT_ReceiverID = IN_SenderID;
	OUT_MessageID = 8;

	switch (IN_MessageID) { //Command type

	case 0: //Ping
		handleAcknowledge(0);
		break;
	case 1: //Battery status
			//Read battery 12 V or 24 V according to variable "IN_Value1".
			//Read percentage between 0 and 99;
			//Split as two numbers;
			//Write to these variables
		//writeOUT_Values(5, 5, IN_Checksum);
		//OUT_MessageID = 1;
		break;
	case 2: //Go to Home
		//writeOUT_Values(2, IN_Value1, IN_Checksum);
		break;
	case 3: //GoTo <location>
		//writeOUT_Values(3, IN_Checksum, 0);
		break;
	case 4: //Move <direction> <amount>
		//writeOUT_Values(4, IN_Checksum, 0);
		handleMove();
		handleAcknowledge(4);
		break;
	}

	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
}

void handleAcknowledge(int messageId) {
	writeOUT_Values(messageId, IN_Checksum, 0);
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
	
	Serial.print("Sending acknowledge command: ");
	Serial.println(calculateMessage(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3, OUT_Checksum));
	sendMessage();
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
	resetMessage();
}

/**
* Calculate the message based on the given attirubtes.
*/
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
 * Handles the ping command. When a ping message is receiver, an acknowledge is returned.
 */
void sendPingCommand() {
	OUT_ReceiverID = 8;
	OUT_MessageID = 0;
	writeOUT_Values(0, 0, 0);
	OUT_Checksum = calculateChecksum(OUT_ReceiverID, OUT_SenderID, OUT_MessageID, OUT_Value1, OUT_Value2, OUT_Value3);
	logMessage("Sending Ping request command");
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
* Calculates the checksum for the given parameters. The checksum is equal
* to the modulo of 8 of the total sum of all parameters.
*
* checksum = (sum(parameters) % 8)
*/
long calculateChecksum(long ReceiverID, long SenderID, long MessageID, long Value1, long Value2, long Value3) {
	long sum = (ReceiverID + SenderID + MessageID + Value1 + Value2 + Value3);
	long modulo = (sum % 8);
	return modulo;
}