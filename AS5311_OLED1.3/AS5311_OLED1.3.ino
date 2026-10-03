//          W.Berggren Netherlands 19dec2025 changed sketch to use 1.3" OLED screen (was 0.9") 
//AS5311 magnetic sensor. Magnetic tape 1mm polewidth (2mm N+S width) works with magnetic strip of SIKO
//works with arduino nano (Chinese clone) connections see below
//3 AA batteries (alkaline or rechargable, both work) on 5V input of arduino and AS5311
//10uF between 3.3V of AS5531 and ground (absolutely necessary to have a capacitor connected)
//200nF between 5V of AS5311 and ground
// progr pin and CSn pin and GND pin of AS5311 to GND and minus battery and GND of arduino
//_____________________________________________________________________________________
//  AS5311                               Arduino                       OLED
//  pin4 = A  -----------------------------D2                           |
//    5  = B  -----------------------------D3                           |
//    8  = GND to GND                       |                           |
//    9  = Prog to GND                      |                           |
//   14  = CSn to GND                       |                           |
//   18  = 3.3V via 10 uF to GND            |                           |
//   19  = 5V to Vin and via 100nF to GND   |                           |
//                                         D4 via reset button to GND   |
//                                         A4--------------------------SDA
//                                         A5--------------------------SCL
//                                         GND to GND                   |
//                                         Vin to Vin                   |
//                                                                     GND to GND
//                                                                     Vcc to Vin
//______________________________________________________________________________ 
      
#include <Wire.h>
#include <Adafruit_GFX.h>
#include<Fonts/FreeSans12pt7b.h>     
#include<Fonts/FreeSans18pt7b.h>     
#include<Adafruit_SH110X.h>          
#define i2c_Address 0x3c             
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1    // No reset pin, shared with microcontroller reset

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  

// AS5311 Quadrature pins
const int pinA = 2; // Must be interrupt-capable pin
const int pinB = 3; // Also interrupt-capable

// Reset (zero) button
const int resetPin = 4;

// Position and zero offset
volatile long position = 0;
volatile long zeroOffset = 0;

// Debounce variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool lastButtonState = HIGH;
//_______________________________________________________________________
void setup() {
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(resetPin, INPUT_PULLUP);

 // Initialize OLED
  display.clearDisplay();                           
  display.begin(i2c_Address, true);                                      
  display.setFont(&FreeSans12pt7b);              
  display.setTextColor(SH110X_WHITE);             

  // Attach interrupts for quadrature decoding
  attachInterrupt(digitalPinToInterrupt(pinA), handleEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), handleEncoderB, CHANGE);
}
//____________________________________________________________________
void loop() {
  // Button debounce & zero logic
  bool buttonState = digitalRead(resetPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    unsigned long currentTime = millis();
    if ((currentTime - lastDebounceTime) > debounceDelay) {
      zeroOffset = position;
      lastDebounceTime = currentTime;
    }
  }
  lastButtonState = buttonState;

  float currentMeasurement;
  float meetwaarde;
  noInterrupts();
  currentMeasurement = position - zeroOffset;
  interrupts();

  // Display measurement
  display.clearDisplay();
  display.setCursor(0, 15);
  //display.setFont(&FreeSans12pt7b);  
  display.print("diametrical:");                       
  display.setCursor(0, 45);  
  //display.setFont(&FreeSans18pt7b);  
  //meetwaarde = currentMeasurement/512;  
  meetwaarde = currentMeasurement/256;                      
  display.print(meetwaarde, 3);
  display.display();

  delay(50);
}

// Quadrature decoding
void handleEncoderA() {
  bool A = digitalRead(pinA);
  bool B = digitalRead(pinB);

  if (A == B) position++;
  else position--;
}

void handleEncoderB() {
  bool A = digitalRead(pinA);
  bool B = digitalRead(pinB);

  if (A != B) position++;
  else position--;
}