#include <EnableInterrupt.h>
#include "traiectorie_arduino.h"

// === PINI MOTOR ===
const int motorLeftPWM = 3;
const int motorLeftDIR = 2;
const int motorRightPWM = 5;
const int motorRightDIR = 4;

// === PINI ENCODERE ===
const int encoderLeft = 12;
const int encoderRight = 13;

volatile long pulsesLeft = 0;
volatile long pulsesRight = 0;

// === PARAMETRI ROBOT ===
const float WHEEL_DIAMETER_CM = 6.5;
const float WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER_CM * 3.1416;
const float ROBOT_WIDTH_CM = 13.0;
const int TICKS_PER_ROTATION = 20;
const float TICKS_PER_CM = TICKS_PER_ROTATION / WHEEL_CIRCUMFERENCE;
const float correction = 1;  // Corecție experimentală
const float correction_lin = 1.5;
const float correction_ang = 1.8;
const float correction_ang_left = 1.05; 

const int PWM_SPEED = 256/2;



float orientareCurenta = 0.0;

void setup() {
  Serial.begin(9600);

  pinMode(motorLeftPWM, OUTPUT);
  pinMode(motorLeftDIR, OUTPUT);
  pinMode(motorRightPWM, OUTPUT);
  pinMode(motorRightDIR, OUTPUT);

  pinMode(encoderLeft, INPUT_PULLUP);
  pinMode(encoderRight, INPUT_PULLUP);

  enableInterrupt(encoderLeft, countLeft, RISING);
  enableInterrupt(encoderRight, countRight, RISING);

  delay(1000);
}

void loop() {
  for (int i = 0; i < numarPuncte - 1; i++) {
    //poz curenta
    float x0 = traseu[i][0];
    float y0 = traseu[i][1];

    //poz dorita
    float x1 = traseu[i + 1][0];
    float y1 = traseu[i + 1][1];

    float dx = x1 - x0;
    float dy = y1 - y0;
    float dist = sqrt(dx * dx + dy * dy);
    float targetAngle = atan2(dy, dx) ;

    float deltaAngle = targetAngle - orientareCurenta;
    while (deltaAngle > 180) deltaAngle -= 360;
    while (deltaAngle < -180) deltaAngle += 360;

    

    rotireLaUnghi(deltaAngle);
    orientareCurenta = targetAngle;
    mersInainte(dist);
    
  }

  opresteMotoarele();
  
  while (true);
}

void mersInainte(float dist_cm) {
  long targetTicks = dist_cm * TICKS_PER_CM * correction_lin;
  resetPulses();
  

  digitalWrite(motorLeftDIR, LOW);   // LOW = înainte
  digitalWrite(motorRightDIR, LOW);

  analogWrite(motorLeftPWM, PWM_SPEED);
  analogWrite(motorRightPWM, PWM_SPEED);

  while (pulsesLeft < targetTicks && pulsesRight < targetTicks) {
    
  }

  opresteMotoarele();

  
}

void rotireLaUnghi(float angle) {
  
  long ticks = (abs(angle)) * ROBOT_WIDTH_CM * TICKS_PER_CM * correction_ang;
  resetPulses();

  if (angle > 0) {
    digitalWrite(motorLeftDIR, HIGH);
    digitalWrite(motorRightDIR, LOW);
    ticks*=correction_ang_left;
  } else {
    digitalWrite(motorLeftDIR, LOW);
    digitalWrite(motorRightDIR, HIGH);
  }

  analogWrite(motorLeftPWM, PWM_SPEED*2-1);
  analogWrite(motorRightPWM, PWM_SPEED*2-1);

  while ((pulsesLeft + pulsesRight) / 2 < ticks) {
    
  }

  opresteMotoarele();

 
}


void opresteMotoarele() {
  analogWrite(motorLeftPWM, 0);
  analogWrite(motorRightPWM, 0);
}

void resetPulses() {
  noInterrupts();
  pulsesLeft = 0;
  pulsesRight = 0;
  interrupts();
}

void countLeft() { pulsesLeft++; }
void countRight() { pulsesRight++; }