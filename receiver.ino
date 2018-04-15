#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>



#define motorCW 30
#define motorCCW 31
#define motorSpeed 33
#define flowControl 30

#define TX HIGH
#define RX LOW
#define FALSE LOW
#define TRUE HIGH

#define HOME 270
#define compassFail 512

volatile int compassOut;
volatile int headingSet;
volatile int positionDifference;
volatile int motorTimer;

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);  //Assign ID to compass sensor

void pollCompass();
void headingSend();

void setup () {
  Serial.begin(9600);
  Serial2.begin(9600);

  pinMode(flowControl, OUTPUT);
  digitalWrite(flowControl, RX);  // Init Transceiver in RX mode
  
  pinMode(motorCW, OUTPUT);       //Init motor control pins & motorStatus
  digitalWrite(motorCW, LOW);
  pinMode(motorCCW, OUTPUT);
  digitalWrite(motorCCW, LOW); 
  pinMode(motorSpeed, OUTPUT); 

  if(!mag.begin()) {              //If Mag sensor fails to init, send error
    compassOut = 512;
    headingSend();
  }

  pollCompass();
  headingSet = compassOut;
}


void pollCompass() 
{ 
  sensors_event_t event;          //Get compass event
  mag.getEvent(&event);

  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  float declinationAngle = 0.1;     //Magnetic declination (-6deg or ~0.1rad)
  heading += declinationAngle;
  
  
  if(heading < 0)                 // Correct for when signs are reversed.
    heading += 2*PI;
    

  if(heading > 2*PI)              // Check for wrap due to addition of declination.
    heading -= 2*PI;
   

  float headingDegrees = heading * 180/M_PI;       // Convert radians to degrees for readability. 

  float headingRound = headingDegrees + 0.5;      //Round to nearest degree
  compassOut = (int)headingRound / 2;              //Divide by 2 to get 8 bit heading   
 
}





void headingSend() {
  
  digitalWrite(flowControl, TX);
  Serial2.write(compassOut);
  digitalWrite(flowControl, RX);
}






void loop () {
  
}

