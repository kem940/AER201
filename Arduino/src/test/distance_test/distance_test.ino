#include <SoftwareSerial.h>
#include "AerDCMotors/AerDCMotors.cpp"

//Convert Rotary Encoder ticks to distance traveled
#define DISTANCE_CONVERSION 1


// Full Batteries
#define PWM_L     160
#define PWM_R     180
#define PWM_ADJ_L 160
#define PWM_ADJ_R 160

//Arduino-PIC Comm
const byte rxPin = 1;
const byte txPin = 0; 
volatile int stat = 0;

//Arduino-PIC Comm
SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin); 

AerDCMotors dc(5, 6, 9, 11);

int line_L = 18;      // A4
int line_R = 19;      // A5
int line_interr = 2;

volatile int curr_pos = 0;
volatile bool isDeploying = false;

void setup() {
  // Initialize DC Motors
  dc.init();
  
  //Arduino-PIC Comm
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT); 
  mySerial.begin(9600); 

  // Initialize IR input sensor pins
  pinMode(line_L,INPUT);
  pinMode(line_R,INPUT);
  pinMode(line_interr, INPUT);
  attachInterrupt(digitalPinToInterrupt(line_interr),line_follow_ISR,CHANGE);
  
  
}

void loop() {
  
  
  dc.forward(PWM_L, PWM_R, true);
  delay(8000);
  dc.stop(PWM_L, PWM_R, false);
  
  detachInterrupt(digitalPinToInterrupt(line_interr));
  while(1);
  
}

void end_operations_test() {
  while(1) {
    // Polling PIC for data from rotary encoders
    if (mySerial.available() > 0) {
      stat =  mySerial.read();
      if (stat == 'E') {
        dc.stop(PWM_L, PWM_R, false);
        detachInterrupt(digitalPinToInterrupt(line_interr));
        break;
      }
      
    }
  }

  while(1);
}

int get_rotary_encoder_data() {
  
  int RE_right = -1;
  mySerial.write('R');
  while(RE_right == -1) {
    if (mySerial.available() > 0) {
      RE_right = mySerial.read();
    }
  }
  
  int RE_left = -1;
  mySerial.write('L');
  while(RE_left == -1) {
    if (mySerial.available() > 0) {
      RE_left = mySerial.read();
    }
  }
  return (RE_left << 8) | RE_right;
  //return ((RE_left << 8) & 0xff00) | (RE_right & 0x00ff);
}

void line_follow_ISR(void) {
  line_follow();
  return;
}
void line_follow() {
  if (isDeploying) {
    return;
  }
  if (digitalRead(line_R) == LOW and digitalRead(line_L) == LOW) {
    // Sensing now lines, go straight
    dc.stop(PWM_L, PWM_R, false);
    delay(100);
    dc.left_wheel_forward(PWM_L);
    dc.right_wheel_forward(PWM_R);
  } else if (digitalRead(line_R) == HIGH and digitalRead(line_L) == LOW) {
    // Sensor over right lane
    dc.right_wheel_stop();
    delay(100);
    dc.left_wheel_forward(PWM_ADJ_L);
  } else if (digitalRead(line_R) == LOW and digitalRead(line_L) == HIGH){
    // Sensor over left lane
    dc.left_wheel_stop();
    delay(100);
    dc.right_wheel_forward(PWM_ADJ_R);
  } else {
    // Sensing both lanes
    dc.stop(PWM_L, PWM_R, true);
  }
}

void wait() {
  while(stat != 'D') {
    if (mySerial.available() > 0) {
      stat = mySerial.read();
    }
  }
  stat = '\0';
}
