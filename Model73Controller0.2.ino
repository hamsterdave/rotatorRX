#include <LiquidCrystal.h>
#define encVCC 21
#define encPinA 20
#define encPinB 19
#define encButton 18
#define encButtonGND 17
#define encGND 16
#define flowControl 13
#define COMPASS_ERROR 500

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile int encoder0Pos = 270;
volatile int encoder0Prev = 0;
volatile int buttonCount = 0;
volatile int buttonPrev = 0;

int headingTX = 0;
int compassDisplay = COMPASS_ERROR;
int compassRX;
int compassPrev;
int headingSet = 270; //Are these being used?
int headingPrev = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(encPinA, INPUT_PULLUP);
  pinMode(encPinB, INPUT_PULLUP);
  pinMode(encButton, INPUT_PULLUP);
  pinMode(encVCC, OUTPUT);
  digitalWrite(encVCC, HIGH);
  pinMode(encGND, OUTPUT);
  digitalWrite(encGND, LOW);
  pinMode(encButtonGND, OUTPUT);
  digitalWrite(encButtonGND, LOW);
  pinMode(flowControl, OUTPUT);
  digitalWrite(flowControl, LOW);
  
  // encoder pin on interrupt 3 (pin 20)
  attachInterrupt(digitalPinToInterrupt(20), doEncoderA, CHANGE);

  // encoder pin on interrupt 4 (pin 19)
  attachInterrupt(digitalPinToInterrupt(19), doEncoderB, CHANGE);

  lcd.begin(16, 2);
  lcdUpdate();
}

void doEncoderA() {
  // look for a low-to-high on channel A
  if (digitalRead(encPinA) == HIGH) {
    // check channel B to see which way encoder is turning
    if (digitalRead(encPinB) == LOW) {
      encoder0Pos = encoder0Pos + 1;         // CW
    } else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  } else { // must be a high-to-low edge on channel A
    // check channel B to see which way encoder is turning
    if (digitalRead(encPinB) == HIGH) {
      encoder0Pos = encoder0Pos + 1;          // CW
    } else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }

  if (encoder0Pos == -1) {
    encoder0Pos = 359;
  }

  if (encoder0Pos == 360) {
    encoder0Pos = 0;
  }
}

void doEncoderB() {
  // look for a low-to-high on channel B
  if (digitalRead(encPinB) == HIGH) {
    // check channel A to see which way encoder is turning
    if (digitalRead(encPinA) == HIGH) {
      encoder0Pos = encoder0Pos + 1;         // CW
    } else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  } else { // Look for a high-to-low on channel B
    // check channel B to see which way encoder is turning
    if (digitalRead(encPinA) == LOW) {
      encoder0Pos = encoder0Pos + 1;          // CW
    } else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }

  if (encoder0Pos == -1) {
    encoder0Pos = 359;
  }

  if (encoder0Pos == 360) {
    encoder0Pos = 0;
  }
 }

void doEncoderPress() {
  static int startTime = 0;
  int onTime = millis();
  static int bouncePrev = bounceTime;
  if (bounceTime - bouncePrev > 50) {
    startTime = 0;
    buttonFlag = true;
  }
}

void lcdUpdate() {
  char fmt[4]; // string to store the formatted heading

  lcd.clear();
   
  if (buttonPrev == buttonCount){
      lcd.setCursor(4, 1);
      lcd.print("*");
  }

  lcd.setCursor(6, 1);

  // print encoder position as a 3-char zero-padded integer
  sprintf(fmt, "%03d", encoder0Pos);
  lcd.print(fmt);
  encoder0Prev = encoder0Pos;

  if (compassDisplay == COMPASS_ERROR){
    lcd.setCursor(4, 0);
    lcd.print("NO  COMM");
  } else {
    lcd.setCursor(6, 0);
    sprintf(fmt, "%03d", compassDisplay);
    lcd.print(fmt);
  }
}

void headingTransmit() {
   headingTX = encoder0Pos / 2;
   digitalWrite(flowControl, HIGH);
   Serial.write(headingTX);
   digitalWrite(flowControl, LOW);
   buttonPrev = buttonCount;
   lcdUpdate();
}

void loop() {
  while (Serial.available()) {
    compassRX = Serial.read();
  }

  if (compassRX != compassPrev || encoder0Pos != encoder0Prev) {
    lcdUpdate();
  }

  if (buttonPrev != buttonCount && compassDisplay != COMPASS_ERROR) {
    headingTransmit();
  }

  if (digitalRead(encButton) == LOW) {
    doEncoderPress();
  }
}
