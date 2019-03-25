 /**
 * @file
 * @author Nkemjika Okeke & Eric Keilty
 * 
 * Date: February 23, 2019 2:46 PM
 * 
 * @brief Entire AutoCone12 Program - Arduino Nano Side
 * 
 * Preconditions:
 * @pre The Co-processor is not driving lines on the UART bus (e.g. the JP_TX
 *      and JP_RX jumpers are removed)
 * @pre PIC-Arduino link switches are enabled (ON) for D1 of the Arduino (the RX
 *      pin). However, make sure that neither of D0 and D1 are enabled (ON) 
 *      while programming the Arduino Nano
 */
// Includes
#include <SoftwareSerial.h>
#include "AerServo/AerServo.cpp"
#include "AerServo/Servo2.cpp"
#include "AerSteppers/AerSteppers.cpp"
#include "AerDCMotors/AerDCMotors.cpp"
#include "Queue/Queue.cpp"

/* Definitions */
//Cone Deployment System Definitions 
#define SENSOR_LEFT         7
#define SENSOR_RIGHT        8
#define SENSOR_CENTRE       3     // Interrupt pin
#define SENSOR_CENTER       3     // Interrupt pin

#define HOLE               10
#define LEFT_CRACK         11
#define RIGHT_CRACK       110
#define CENTRE_CRACK      111
#define CENTER_CRACK      111

//Line Following Sensors Definitions
#define line_L             18    // A4
#define line_R             19    // A5
#define line_interr         2     // Interrupt pin

//Convert Rotary Encoder ticks to distance traveled
#define DISTANCE_CONVERSION 1

// Full Batteries
#define PWM_L     180
#define PWM_R     180
#define PWM_ADJ   150
/*
// Half Dead Batteries
#define PWM_L     215
#define PWM_R     230
#define PWM_ADJ   170
*/

//Read-only (Arduino-PIC Comm)
const byte rxPin = 1;
const byte txPin = 0; 
volatile int stat = 0;

//Global variables for primary initializing 
volatile int curr_pos = 0;
volatile int total_number_of_cones = 13;
volatile int number_of_cones_deployed = 0;

//Detection ISR Variables 
volatile bool isDeploying = false;
volatile int last_obstruction[2] = {0, 0};
volatile bool isPreviouslyDeployed = false;

//Data to send to PIC at end of operation
volatile int holes[20];
volatile int hole_num = 0;
volatile int cracks[20];
volatile int crack_num = 0;

//Cone Deployment Functions
void detection_ISR();
void cone_deployment();
void hole_deploy();
void left_crack_deploy();
void right_crack_deploy();
void centre_crack_deploy();
void center_crack_deploy();

//Line Following Functions
void line_follow_ISR();
void line_follow();


/* Object Initializations */
//Arduino-PIC Comm
SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin); 

//Initialize Queues
Queue HC_q(8);
Queue HC_Dist_q(8);

//Initialize DC Motors
// AerDCMotors(int pinL1, int pinL2, int pinR1, int pinR2)
AerDCMotors dc(5, 6, 9, 11);

//Initialize Servo
AerServo servo(10);

//Initialize Steppers, pins A0:3
AerSteppers stepp(14, 15, 16, 17); 

void setup() {

  // Initialize internal variables of classes
  dc.init();
  dc.stop();
  servo.init(); 
  servo.stop();
  servo.setCurr_pos(8.5);
  stepp.init();
  HC_q.init();

  //Arduino-PIC Comm
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT); 
  mySerial.begin(9600); 

  // Getting starting instruction
  while (1) {
    if (mySerial.available() > 0) {
      stat = mySerial.read();
    }

    if (4 <= stat and stat <= 12) {
      total_number_of_cones = (int) stat;
    }
  
    if (stat == 100) {
      if (4 <= total_number_of_cones and total_number_of_cones <= 12) {
        break;
      } else {
        // Not sure if we want to account for this case
        while(1);
      }
    }
  }

  //Set Interrupt pins
  pinMode(SENSOR_CENTRE, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR_CENTRE), detection_ISR, CHANGE);

  pinMode(line_L,INPUT);
  pinMode(line_R,INPUT);
  pinMode(line_interr, INPUT);
  attachInterrupt(digitalPinToInterrupt(line_interr),line_follow_ISR,CHANGE);
  
  delay(1000);

  // Operations begin by robot moving forward
  dc.left_wheel_forward(PWM_L);
  dc.right_wheel_forward(PWM_R);
  
}

void loop() {

  // Core operations
  while(1) {
    if (mySerial.available() > 0) {
       curr_pos =  mySerial.read() * DISTANCE_CONVERSION;
    }
    
    if (number_of_cones_deployed >= total_number_of_cones or curr_pos >= 4000000000) {
      dc.stop();
      mySerial.write('A');
      break;
    }
    
    if (HC_q.isEmpty() == false){
      isDeploying = true;
      cone_deployment();
      isDeploying = false;
    }
  }

  // Returning to startline
  while(1) {
    dc.uturn_right(255, 255, 255);
    dc.left_wheel_forward(PWM_L);
    dc.right_wheel_forward(PWM_R);
    curr_pos = 0;
    dc.left_wheel_forward(PWM_L);
    dc.right_wheel_forward(PWM_R);
    while (curr_pos < 400) {
      curr_pos =  mySerial.read() * DISTANCE_CONVERSION;
    }
    dc.uturn_right(255, 255, 255);
    dc.stop();
    break;
  }

  while(1);
  
}

/* Cone Deployment Functions */
unsigned long lastInterrupt = 0;
void detection_ISR(){
  
  // To prevent repeated input due to bad sensor signal
  if (millis() - lastInterrupt > 200) {
    
    // Updating curr_pos to get accurate location of obstruction
    /*
    if (mySerial.available() > 0) {
      curr_pos = mySerial.read();
    }
    */
    
    //int obstruction = 100*digitalRead(SENSOR_RIGHT) + 10*digitalRead(SENSOR_CENTRE) + digitalRead(SENSOR_LEFT);
    int obstruction = 0;
    if (digitalRead(SENSOR_CENTRE) == LOW) {
      return;
    } else if (digitalRead(SENSOR_RIGHT) == HIGH and digitalRead(SENSOR_LEFT) == HIGH) {
      // Center Crack
      obstruction = 111;
      cracks[crack_num] = curr_pos;
      mySerial.write('C'); 
    } else if (digitalRead(SENSOR_RIGHT) == HIGH) {
       // Right Crack
       obstruction = 110;
       cracks[crack_num] = curr_pos;
       mySerial.write('C'); 
    } else if (digitalRead(SENSOR_LEFT) == HIGH) {
      // Left Crack
      obstruction = 11;
      cracks[crack_num] = curr_pos;
      mySerial.write('C'); 
    } else if (digitalRead(SENSOR_RIGHT) == LOW and digitalRead(SENSOR_LEFT) == LOW) {
      // Hole
      obstruction = 10;
      holes[hole_num] = curr_pos;
      mySerial.write('H');
    } else {
      // idk
      return;
    }
    
    dc_stop();
    HC_q.enq(obstruction);
    HC_Dist_q.enq(curr_pos);
  }
  lastInterrupt = millis();
  
  return;
}

void dc_stop() {
  dc.stop();
}

void cone_deployment(){
  
  isDeploying = false;
  dc.left_wheel_forward(PWM_L);
  dc.right_wheel_forward(PWM_R);
  delay(1000);
  dc.stop();
  isDeploying = true;

  int obstruction = HC_q.deq();
  int pos = HC_Dist_q.deq();
  bool shouldCurrentlyDeploy = shouldDeploy(last_obstruction[0], obstruction, pos-last_obstruction[1]);

  if (!isPreviouslyDeployed or (isPreviouslyDeployed and shouldCurrentlyDeploy)) {
    
    switch(obstruction) {
      case HOLE:
        hole_deploy();
        number_of_cones_deployed += 1;
        break;
      case LEFT_CRACK:
        left_crack_deploy();
        number_of_cones_deployed += 2;
        break;
      case RIGHT_CRACK:
        right_crack_deploy();
        number_of_cones_deployed += 2;
        break;
      case CENTRE_CRACK:
        centre_crack_deploy();
        number_of_cones_deployed += 2;
        break;
      default:
        break;
    }
    
    // putting servo in neutral position
    //servo.to_middle();
    
  }
  
  last_obstruction[0] = obstruction;
  last_obstruction[1] = pos;
  isPreviouslyDeployed = (!isPreviouslyDeployed or (isPreviouslyDeployed and shouldCurrentlyDeploy));

  // Begin moving after done deployment
  dc.left_wheel_forward(PWM_L);
  dc.right_wheel_forward(PWM_R);
  
  return;
}

bool shouldDeploy(int prev, int curr, float diff) {

  if (diff > 20.0) {
    // hole, then crack, >20cm  or
    // crack, then hole, >20cm
    return true;
  }
  
  if (prev == HOLE) {
    if (curr == HOLE) {
      if (diff < 15.0) {
        // hole, then hole, <15cm
        return false;
      }
      // hole, then hole, >15cm
      return true;
    }
    // hole, then crack <20cm
    return false;
  } else { /*prev == CRACK*/
    if (curr != HOLE) { /*curr == CRACK*/
      if (diff < 10) {
        // crack, then crack <10cm
        return false;
      }
      // crack, then crack, >10cm
      return true;
    }
    // crack, then hole, <20cm
    return false;
  }
}

void hole_deploy() {
  servo.to_middle();
  stepp.drop_cone();
  return;
}
void left_crack_deploy() {
  // Cone 1
  servo.move_to_time(-2500);
  servo.move_to_time(300);
  stepp.drop_cone();
  // Cone 2
  servo.move_to_time(1500);
  stepp.drop_cone();
  // Move back to center
  servo.setCurr_pos(10);
}
void right_crack_deploy() {
  // Cone 1
  servo.move_to_time(2000);
  servo.move_to_time(-400);
  stepp.drop_cone();
  // Cone 2
  servo.move_to_time(-2100);
  stepp.drop_cone();
  servo.setCurr_pos(7);
}
void centre_crack_deploy() {
  // Cone 1
  servo.move_to_time(-2500);
  servo.move_to_time(600);
  stepp.drop_cone();
  // Cone 2
  servo.move_to_time(3500);
  servo.move_to_time(-500);
  stepp.drop_cone();
  servo.setCurr_pos(13);
}
void center_crack_deploy() {
  centre_crack_deploy();
}

/* Line Following Code */
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
    dc.left_wheel_forward(PWM_L);
    dc.right_wheel_forward(PWM_R);
  } else if (digitalRead(line_R) == HIGH and digitalRead(line_L) == LOW) {
    // Sensor over right lane
    dc.left_wheel_forward(PWM_ADJ);
    dc.right_wheel_stop();
  } else if (digitalRead(line_R) == LOW and digitalRead(line_L) == HIGH){
    // Sensor over left lane
    dc.left_wheel_stop();
    dc.right_wheel_forward(PWM_ADJ);
  } else {
    // Sensing both lanes
    dc.stop();
  }
}
