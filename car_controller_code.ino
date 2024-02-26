
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

//initialising variables

//reads an analog voltage representing the positon of the joystick
int XPIN=A0;
int YPIN=A3;

//buffer for calculating the average x and y position
int XVal[5]={0,0,0,0,0};
int YVal[5]={0,0,0,0,0};
//initialising average to zero
int avgX=0;
int avgY=0;
//represents the center point of the joystick, which is the referece for which other joystick positons are calculated
int X0=0;
int Y0=0;
//buttons that control the forklift, only button 1 and 2 were used in the final design
int button1=7;
int button2=6;
int button3=5;
int button4=4;

// buffer containing the data to be sent to the car
uint8_t sendToCar[8]; //M1,M2,button1,button2,button3,button4,M1 + or -,M2 + or -
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

//sets up a connection between car an controller
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_NRF24 driver;
// RH_NRF24 driver(8, 7);   // For RFM73 on Anarduino Mini

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
 pinMode(button1,INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);
  pinMode(button4,INPUT); 
  //initialise neutral value (controller has not moved)
  //loops a couple of times to let the neutral values settle
  for (int i=0;i<5;i++)
  {
    X0 = analogRead(XPIN);
    Y0 = analogRead(YPIN);
    delay(40);
  }  
}


void loop()
{
  //not the prettiest code but it gets the job done.
  //shifts XVal and YVal values down an index
  for (int i=0;i<4;i++)
  {
    XVal[i]=XVal[i+1];
    YVal[i]=YVal[i+1];
  }
  //obtains new controller data
  XVal[4]=analogRead(XPIN);
  YVal[4]=analogRead(YPIN);
  //reset average values
    avgX=0;
    avgY=0;
    //calculate average
  for (int i=0;i<5;i++)
  {
    avgX=avgX+XVal[i];
    avgY=avgY+YVal[i];
  }
  avgX=avgX/5;
  avgY=avgY/5;

  //map the x an y positions of the controller to pwm values
  avgX=-map(avgX-X0,0-X0,1028-X0,-255,255);
  avgY=-map(avgY-Y0,0-Y0,1028-Y0,-255,255);

 //tells how much each motor on the arduino car should move, subtracting y-x allows for turning by making one motor slower than the other.
 //Motor one was stronger than Motor 2 so it is scaled down a bit. Scaling values were found through testing
  sendToCar[1]=abs(avgY-(0.5)*avgX);
  sendToCar[0]= abs(-(0.88)*avgY-(0.35)*avgX);
  //ensures the pwm value given to the motor won't be above 255
  //doesn't really dip below 0 so a check for that was ommited.
  if (sendToCar[0]>255)
  {
    sendToCar[0]=255;
  }
  if (sendToCar[1]>255)
  {
    sendToCar[1]=255;
  }

  //determining if the motor should clockwise or counter clockwise depending on if the sign of sendToCar prior to abs is + or -
  //send to Car is an unsigned int so can't be negative since I want it to have values from 0-255
  if (avgY-0.5*avgX>0)
  {
    sendToCar[6]=1;
  }
  else if (avgY-0.5*avgX<=0)
  {
    sendToCar[6]=2;
  }
  
  if (-(0.88)*avgY-(0.35)*avgX>0)
  {
    sendToCar[7]=1;
  }
  if (-(0.88)*avgY-(0.35)*avgX<=0)
  {
    sendToCar[7]=2;
  }
  //getting button values
  sendToCar[2]=digitalRead(button1);
  sendToCar[3]=digitalRead(button2);
  sendToCar[4]=digitalRead(button3);
  sendToCar[5]=digitalRead(button4);
  // Send a message to manager_server
  if (manager.sendtoWait(sendToCar, sizeof(sendToCar), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      /*
       * for debugging:
        Serial.print("X:");
        Serial.print(avgX);
        Serial.print("     Y:");
        Serial.print(avgY);
        Serial.print("     motor1:");
        Serial.print(sendToCar[0]);
        Serial.print("  M1Sign:");
        Serial.print(sendToCar[6]);
        Serial.print("     motor2:");
        Serial.print(sendToCar[1]);
        Serial.print("  M2Sign:");
        Serial.print(sendToCar[7]);
          Serial.print('\n');
          */ 
    }
    else
    {
      // debugging: Serial.println("No reply, is nrf24_reliable_datagram_server running?");
    }
  }
  else
    //debugging: Serial.println("sendtoWait failed");
    
  delay(20);
}
