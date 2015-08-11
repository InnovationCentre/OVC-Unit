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
const byte SPEED1  =            (byte)0x00;                        // Byte to send speed to first motor
const byte SPEED2   =           0x01;                              // Byte to send speed to second motor
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

void setup () {
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
    Serial.print("Received message: ");
    Serial.println( mySwitch.getReceivedValue() );
    readMessage();

    if (IN_Checksum == calculateChecksum(IN_ReceiverID, IN_SenderID, IN_MessageID, IN_Value1, IN_Value2, IN_Value3)) { //Message is valid
      Serial.print("Message ");
      Serial.print(Message);
      Serial.println(" is a valid message");

      if (IN_ReceiverID == robotId || IN_ReceiverID == broadcastId) { //Message is send to this robot

        OUT_ReceiverID = IN_SenderID;
        OUT_MessageID = 8;
        
        switch (IN_MessageID)  { //Command type

        case 0: //Ping
          writeOUT_Values(0,IN_Checksum,0);
          break;
        case 1: //Battery status
          //Read battery 12 V or 24 V according to variable "IN_Value1".
          //Read percentage between 0 and 99;
          //Split as two numbers;
          //Write to these variables
          writeOUT_Values(5,5,IN_Checksum);
          OUT_MessageID = 1;
          break;
        case 2: //Go to Home
          writeOUT_Values(2,IN_Value1,IN_Checksum);
          break;
        case 3: //GoTo <location>
          writeOUT_Values(3,IN_Checksum,0);
          break;
        case 4: //Move <direction> <amount>
          writeOUT_Values(4,IN_Checksum,0);
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

