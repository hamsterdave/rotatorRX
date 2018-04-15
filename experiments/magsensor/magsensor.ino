#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>



#define motorCW 30
#define motorCCW 31
#define motorSpeed 33
#define serial2Control 3
#define FAST 2
#define SLOW 1
#define OFF 0
#define FALSE LOW
#define TRUE HIGH

#define TX HIGH
#define RX LOW


volatile uint8_t motorStatus;
volatile int motorTimer;
volatile uint8_t headingTX;
volatile uint8_t positionRX;
volatile int positionDisplay;

volatile bool headingChange;
volatile bool motorStateChange;

/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

void readCompass();
void headingSend();
void positionReceive();
void directionControl();

void setup(void) 
{
 
  headingTX = 0;
  positionRX = 0;
  positionDisplay = 0;
  motorStatus = 0;
  motorTimer = 0;
  headingChange = FALSE;
  motorStateChange = FALSE;
  
  pinMode(serial2Control, OUTPUT);
  digitalWrite(serial2Control, RX);  // Init Transceiver in RX mode
  
  pinMode(motorCW, OUTPUT);  //Init motor control pins & motorStatus
  digitalWrite(motorCW, LOW);
  pinMode(motorCCW, OUTPUT);
  digitalWrite(motorCCW, LOW); 
  pinMode(motorSpeed, OUTPUT);
 
  motorStatus = 0;

  Serial.begin(9600);
  Serial2.begin(9600); 
  
  /* Initialise the sensor */
  if(!mag.begin()) {        //If Mag sensor fails to init, send error
  digitalWrite(serial2Control, TX);
  Serial2.write("510");      //Controller will display 510 heading
  digitalWrite(serial2Control, RX);
  }

  readCompass();
  
}

void readCompass() 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.22;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

  float headingRound = headingDegrees + 0.5;
  headingTX = (int)headingRound / 2;   
 
  headingSend();
}


void headingSend() {
  
  digitalWrite(serial2Control, TX);
  Serial2.write(headingTX);
  digitalWrite(serial2Control, RX);
}

void positionReceive() {
  if (motorStatus != 0) {
    digitalWrite(motorCW, LOW);   //if motor in motion when new heading RX
    digitalWrite(motorCCW, LOW);  //stop motor (ADD DELAY)
  }
  while (Serial2.available()) {
    positionRX = Serial2.read();
  }
  directionControl();
}




void directionControl() {
	static uint8_t positionDifference = 0;
  positionDifference = headingTX - positionRX;
  if (positionDifference != 0){
  
  if (positionDifference > 10) { 
    digitalWrite(motorCCW, HIGH);
    digitalWrite(motorSpeed, HIGH);
  }

  if (positionDifference > 2 && positionDifference < 10) {
    digitalWrite(motorCCW, HIGH);
    digitalWrite(motorSpeed, LOW);
  }

  if (positionDifference < -10) {
    digitalWrite(motorCW, HIGH);
    digitalWrite(motorCW, HIGH);
  }

  if (positionDifference < -2 && positionDifference > -10) {
    digitalWrite(motorCW, HIGH);
    digitalWrite(motorCW, LOW);
  }
  }

}




void loop(){
}


