#include "AerDCMotors/AerDCMotors.cpp"

#define FORWARD 0
#define BACKWARD 1
#define SLOW  2
#define STOP 3

#define RIGHT true
#define LEFT false

//Initiate DC Motors
AerDCMotors dc(7, 8, 9, 10);

void setup() {
  dc.init();
}

void loop() {
  
  while(1); 

}

void wheel_test(AerDCMotors dc, int dir) {
  if (dir == FORWARD) {
    dc.forward();
  } else if (dir == BACKWARD) {
    dc.backward();
  } else if (dir == SLOW) {
    dc.slow();
  } else {
    dc.stop();
  }
}

void turning_self_consistency_test(AerDCMotors dc, int isRight) {
  if (isRight) {
    dc.u_turn_right();
    dc.u_turn_right();
  } else {
    dc.u_turn_left();
    dc.u_turn_left();
  }
}

//Line following with rotary encoders
//  I'm guessing with this method, I'm not really sure how it's going to be implemented
//  Again, at this point it's more for reference to show you what the logic should be
//  based on the inputs
void line_follow(AerDCMotors dc, int irL, int irR) {

  if (irL == HIGH) {
    dc.left_wheel_forward();
  } else {
    dc.left_wheel_stop();
  }

  if (irR == HIGH) {
    dc.right_wheel_forward();
  } else {
    dc.right_wheel_stop();
  }

  // The logic more explicitly written
  /*
  if(irL == HIGH && irR == HIGH) {
    dc.left_wheel_forward();
    dc.right_wheel_forward();
  } else if(irL == HIGH && irR == LOW) {
    dc.left_wheel_forward();
    dc.right_wheel_stop();
  } else if (irL == LOW && irR == HIGH) {
    dc.left_wheel_stop();
    dc.right_wheel_forward();
  } else {
    // something probably went wrong
    dc.left_wheel_stop();
    dc.right_wheel_stop();
  }
  */
}