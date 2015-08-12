/**
 * Enable the motors to drive at a certain speed. Speed value of 128 stops the motors,
 * a value of 255 isfull forward and a value of 0 is full reverse. The motors are
 * disables automatically after 2 seconds.
 *
 * @param address Address of the wheel
 * @param driveSpeed Speed of the motor
 */
void drive(byte address, int driveSpeed)
{
	Wire.beginTransmission(MD25ADDRESS);
	Wire.write(address);
	Wire.write(driveSpeed);
	Wire.endTransmission();
}

void driveRight(int driveSpeed) {
	drive(SPEED1, driveSpeed);
}

void driveLeft(int driveSpeed) {
	drive(SPEED2, driveSpeed);
}

void drive(int leftSpeed, int rightSpeed) {
	driveRight(rightSpeed);
	driveLeft(leftSpeed);
}

/**
 * Drive straight forward for a certain distance
 *
 * @param distance Distance in milimeters
 */
void DistanceDrive(int distance) 
{
	int EncCalc = PPT * (distance / (PI * WheelDiameter));
	encodeReset();
	Serial.println(EncCalc);
	do { 
		drive(DriveSpeed, DriveSpeed);
		encoder(ENCODERONE);
		encoder(ENCODERTWO);
	} while (EncCalc > encoder(ENCODERTWO) || EncCalc > encoder(ENCODERONE));
	stopMotor();
}

/**
 * Stops the motors instantly.
 */
void stopMotor() {
	drive(StopSpeed, StopSpeed);
}

/**
 * Turn for a certain angle
 *
 * @param degree Angle degree
 * @param dir Direction to turn. 1 is right, 0 is left.
 */
void Turn(int degree, int dir)
{
	unsigned int TurnCalc = PPT * (((degree * PI * WheelWidth) / 360) / (PI * WheelDiameter));
	encodeReset();
	Serial.println(TurnCalc);
	if (dir == 1)
	{
		do {                                                       // Start loop to drive motors forward
			drive(128 + TurnConst, 128 - TurnConst);
			encoder(ENCODERONE);    
			encoder(ENCODERTWO);                  
		} while (TurnCalc >= encoder(ENCODERONE) || -TurnCalc <= encoder(ENCODERTWO));
	}
	else if (dir == 0)
	{
		do {                                                       // Start loop to drive motors forward
			drive(128 - TurnConst, 128 + TurnConst);
			encoder(ENCODERONE);
			encoder(ENCODERTWO);   
		} while (TurnCalc >= encoder(ENCODERTWO) || -TurnCalc <= encoder(ENCODERONE));
	}
	stopMotor();
}

void SpeedControlReset() {									// This function disables the speedcontrol
	writeToAddress(0x30);
}

void SpeedControlEnable()
{ // This function enables the speedcontrol
	writeToAddress(0x31);
}

void encodeReset() {										// This function resets the encoder values to 0
	writeToAddress(0x20); 
}

void writeToAddress(byte message) {
	Wire.beginTransmission(MD25ADDRESS);
	Wire.write(CMD);
	Wire.write(message); 
	Wire.endTransmission();
	delay(100);
}

long encoder(byte encoderAddress) {							// Function to read and display value of encoder 1 as a long
	Wire.beginTransmission(MD25ADDRESS);					// Send byte to get a reading from encoder 1
	Wire.write(encoderAddress);
	Wire.endTransmission();

	Wire.requestFrom(MD25ADDRESS, 4);						// Request 4 bytes from MD25
	while (Wire.available() < 4);							// Wait for 4 bytes to arrive
	long poss1 = Wire.read();								// First byte for encoder 1, HH.
	poss1 <<= 8;
	poss1 += Wire.read();									// Second byte for encoder 1, HL
	poss1 <<= 8;
	poss1 += Wire.read();									// Third byte for encoder 1, LH
	poss1 <<= 8;
	poss1 += Wire.read();									// Fourth byte for encoder 1, LL
	Serial.println();
	Serial.print("Encoder ");
	Serial.print(encoderAddress);
	Serial.print(" = ");
	Serial.println(poss1, DEC);
	delay(50);												// Wait for everything to make sure everything is sent

	return (poss1);
}

void volts()
{ // Function to read and display battery volts as a single byte
	Wire.beginTransmission(MD25ADDRESS);                      // Send byte to read volts
	Wire.write(VOLTREAD);
	Wire.endTransmission();

	Wire.requestFrom(MD25ADDRESS, 1);
	while (Wire.available() < 1);
	int batteryVolts = Wire.read();

	Serial.print("Battery v = ");
	Serial.print(batteryVolts / 10, DEC);                     // Print batery volts / 10 to give you the number of whole volts
	Serial.print(".");                                        // As arduino has truncates floating point numbers we print a . here to act as a decimal point
	Serial.print(batteryVolts % 10, DEC);                     // Prints Battery volts % 10 to give us the

	delay(50);                                                // Wait to make sure everything is sent
}
