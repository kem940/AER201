#include <SoftwareSerial.h>
#include "AerDCMotors/AerDCMotors.cpp"

#define RIGHT true
#define LEFT false

#define PWM_L     245
#define PWM_R     255

#define DISTANCE_CONVERSION 1

volatile int curr_pos = 0;
volatile int stat = 0;
const byte rxPin = 1;
const byte txPin = 0; 

//Initiate DC Motors
// AerDCMotors(int pinR2, int pinR1, int pinL2, int pinL1)
//    as in the order it goes on the integrated H-bridge
AerDCMotors dc(5, 6, 9, 11);

//Arduino-PIC Comm
SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

void setup() {
  // Initialize DC Motors
  dc.init();

  //Arduino-PIC Comm
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT); 
  mySerial.begin(9600);
}

int val = 0;

void loop() {
  wheel_test(dc, 255);
  while(1);
}

/*
void PWM_test_turn(AerDCMotors dc){
  
  dc.forward(255); 
  delay(500); 
  
  for (int i=200;i<230;i++){
    dc.forward(230);
    delay(50); 
  }
  
  delay(2000); 
  dc.stop();
  
}


void right_turn(AerDCMotors dc) {
  dc.left_wheel_forward(255);
  dc.right_wheel_backward(255);
  delay(1200);
  dc.stop();
  delay(1200);
  dc.forward(255); 
  delay(1200);
  dc.stop();
  delay(1200);
  dc.left_wheel_forward(255);
  dc.right_wheel_backward(255);
  delay(1200);
  dc.stop();
}
*/

void wheel_test(AerDCMotors dc, int pwm_val) {
  
  delay(1000);
  dc.forward(pwm_val, pwm_val);
  delay(1000);
  dc.stop();
  
  delay(1000);
  dc.backward(pwm_val);
  delay(1000);
  dc.stop();

  delay(1000);
  dc.left_wheel_forward(pwm_val);
  delay(1000);
  dc.stop();

  delay(1000);
  dc.right_wheel_forward(pwm_val);
  delay(1000);
  dc.stop();

  delay(1000);
  dc.left_wheel_backward(pwm_val);
  delay(1000);
  dc.stop();

  delay(1000);
  dc.right_wheel_backward(pwm_val);
  delay(1000);
  dc.stop();
}
