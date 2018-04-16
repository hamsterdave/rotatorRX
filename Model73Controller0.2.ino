#include <LiquidCrystal.h>
#define encVCC 21
#define encPinA 20
#define encPinB 19
#define encButton 18
#define encButtonGND 17
#define encGND 16
#define flowControl 13

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile int encoder0Pos = 0;
volatile int encoder0Prev = 0;
volatile int buttonCount = 0;
volatile int buttonPrev = 0;
volatile int bounceTime = 0;
volatile int bouncePrev = 0;

int headingTX = 0;
int compassDisplay;
int compassRX;
int compassPrev;
int headingSet = 270; //Are these being used?
int headingPrev = 0;

void setup() {
  compassDisplay = 500;
  encoderPos = 270;

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

  // Encoder Button on interrupt 5 (pin 18)
  attachInterrupt(digitalPinToInterrupt(18), doEncoderPress, LOW);

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
  bounceTime = millis();
  if (bounceTime - bouncePrev > 50) {
    bouncePrev = bounceTime;
    buttonCount++;
  }
}

void lcdUpdate() {
  lcd.clear();
   
  if (buttonPrev == buttonCount){
      lcd.setCursor(4, 1);
      lcd.print("*");
      lcd.setCursor(6, 1);
  } else {
    lcd.setCursor(6, 1);
  }

  char n[4]; // string to store the formatted number
  sprintf(n, "%03d", encoder0Pos); // print the value of x formatted as a 3-character zero-padded decimal integer to the string "n"
  lcd.print(n); // print the string "n" to the lcd
  encoder0Prev = encoder0Pos;

  if (compassDisplay == 500){
    lcd.setCursor(4, 0);
    lcd.print("NO  COMM");
  } else {
    char c[4];
    lcd.setCursor(6, 0);
    sprintf(c, "%03d", compassDisplay);
    lcd.print(c);
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

  if (compassRX != compassPrev) {
    if (encoder0Pos != encoder0Prev) {
            lcdUpdate();
    }
  }

  if (buttonPrev != buttonCount) {
    if (compassDisplay != 500) {
      headingTransmit();
    }
  }
}
