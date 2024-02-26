/*Code Author: Kasun Fernando 
 * Code purpose: This is the code for controlling the forklift car
 * Last Modified: 20/02/2024
 */
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_NRF24 driver;
// RH_NRF24 driver(8, 7);   // For RFM73 on Anarduino Mini

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);
// assigning the pins 
int motor2PWM=3;
int motor2Anti=4;
int motor2Clok=5;
int motor1Anti = 6;
int motor1Clok = 7;
int motor1PWM = 9;

void setup() 
{
  //initialising pins
  pinMode(motor1PWM,OUTPUT);
  pinMode(motor2PWM,OUTPUT);
  pinMode(motor2Anti,OUTPUT);
  pinMode(motor2Clok,OUTPUT);
  pinMode(motor1Anti,OUTPUT);
  pinMode(motor1Clok,OUTPUT);

  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A4,OUTPUT);
  pinMode(A5,OUTPUT);
  Serial.begin(9600);
  if (!manager.init())
  {
    // debugging: Serial.println("init failed");
  }
  else
  {
    //debugging: Serial.println("init worked");
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
}

// debugging, package sent to controller to tell it that the car recieved the data: uint8_t data[] = "joystick data recieved";
// buffer that will recieve data
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    int8_t len = sizeof(buf);
    int8_t from;
    // if the buffer receives data
    if (manager.recvfromAck(buf, &len, &from))
    {
      //assign pwm values to motor, controls speed
      analogWrite(motor1PWM,buf[0]);
      analogWrite(motor2PWM,buf[1]);
      // reading the sign of the motor to chekc if it should spin clockwise or anticlockwise
      if (buf[6]==1)
      {
        digitalWrite(motor1Clok,LOW);
        digitalWrite(motor1Anti,HIGH);
      }
      else if (buf[6]==2)
      {
        digitalWrite(motor1Clok,HIGH);
        digitalWrite(motor1Anti,LOW);
      }
      
      if (buf[7]==1)
      {
        digitalWrite(motor2Clok,HIGH);
        digitalWrite(motor2Anti,LOW);
      }
      else if (buf[7]==2)
      {
        digitalWrite(motor2Clok,LOW);
        digitalWrite(motor2Anti,HIGH);
      }

      //code to control the motor attached to the forklift, just assigning it a value of 255 or 0
      if (buf[2]==1 && buf[3]==0) //go down
      {
        digitalWrite(A1,HIGH);
        digitalWrite(A2,LOW);
        digitalWrite(A4,LOW);
        digitalWrite(A5,HIGH);
      }
      else if (buf[3]==1 && buf[2]==0)//go up
      {
        digitalWrite(A1,LOW);
        digitalWrite(A2,HIGH);
        digitalWrite(A4,HIGH);
        digitalWrite(A5,LOW);
      }
      else
      {
        digitalWrite(A1,LOW);
        digitalWrite(A2,LOW);
        digitalWrite(A4,LOW);
        digitalWrite(A5,LOW);
      }
      
      /*
       * for debugging
      Serial.print(buf[2]);
      Serial.print("    ");
      Serial.print(buf[3]);
      Serial.print("    ");
      Serial.print(buf[4]);
      Serial.print("    ");
      Serial.print(buf[5]);
      Serial.print("\n");
      delay(50);
      */
      if (!manager.sendtoWait(data, sizeof(data), from)){
        // for debugging, its for if the car can't identify the reciever: Serial.println("sendtoWait failed");
    }
  }
}
