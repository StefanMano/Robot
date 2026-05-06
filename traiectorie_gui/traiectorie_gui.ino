#include <EnableInterrupt.h>
#include "traiectorie_arduino.h"

// === PINI MOTOR ===
const int motorLeftPWM = 5;
const int motorLeftDIR = 2;
const int motorRightPWM = 3;
const int motorRightDIR = 4;

// === PINI ENCODERE ===
const int encoderLeft = 13;
const int encoderRight = 10;

volatile long pulsesLeft = 0;
volatile long pulsesRight = 0;

// === PARAMETRI ROBOT ===
const float WHEEL_DIAMETER_CM = 6.5;
const float WHEEL_CIRCUMFERENCE = WHEEL_DIAMETER_CM * 3.1416;
const float ROBOT_WIDTH_CM = 13.0;
const int TICKS_PER_ROTATION = 20;
const float TICKS_PER_CM = TICKS_PER_ROTATION / WHEEL_CIRCUMFERENCE;
const float correction = 1;  // Corecție experimentală

const int PWM_SPEED = 150;

float orientareCurenta = 0.0;  // în grade

// === VARIABILE ODOMETRIE ===
float x = 0.0;
float y = 0.0;
float theta = 0.0;  // în radiani

// === PID pentru mers inainte ===
float kp = 0.6;   // Doar componenta P activă inițial
float ki = 0.0;
float kd = 0.0;
float integral = 0.0;
float previous_error = 0.0;

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
    float x0 = traseu[i][0];
    float y0 = traseu[i][1];
    float x1 = traseu[i + 1][0];
    float y1 = traseu[i + 1][1];

    float dx = x1 - x0;
    float dy = y1 - y0;
    float dist = sqrt(dx * dx + dy * dy);
    float targetAngle = atan2(dy, dx) * 180.0 / 3.1416;

    float deltaAngle = targetAngle - orientareCurenta;
    while (deltaAngle > 180) deltaAngle -= 360;
    while (deltaAngle < -180) deltaAngle += 360;

    Serial.print("De la (");
    Serial.print(x0); Serial.print(", "); Serial.print(y0);
    Serial.print(") la (");
    Serial.print(x1); Serial.print(", "); Serial.print(y1);
    Serial.print(") → d=");
    Serial.print(dist); Serial.print("cm, Δθ=");
    Serial.println(deltaAngle);

    rotireLaUnghi(deltaAngle);
    orientareCurenta = targetAngle;
    mersInainte(dist);
    delay(300);
  }

  opresteMotoarele();
  Serial.println("Traiectorie completă!");
  while (true);
}

void mersInainte(float dist_cm) {
  long targetTicks = dist_cm * TICKS_PER_CM * correction;
  resetPulses();
  integral = 0;
  previous_error = 0;

  digitalWrite(motorLeftDIR, LOW);
  digitalWrite(motorRightDIR, LOW);

  while ((pulsesLeft + pulsesRight) / 2 < targetTicks) {
    long error = pulsesLeft - pulsesRight;

    integral += error;
    float derivative = error - previous_error;
    float output = kp * error + ki * integral + kd * derivative;
    previous_error = error;

    int pwmLeft = PWM_SPEED - output;
    int pwmRight = PWM_SPEED + output;

    pwmLeft = constrain(pwmLeft, 0, 255);
    pwmRight = constrain(pwmRight, 0, 255);

    analogWrite(motorLeftPWM, pwmLeft);
    analogWrite(motorRightPWM, pwmRight);

    Serial.print("L="); Serial.print(pulsesLeft);
    Serial.print(" R="); Serial.print(pulsesRight);
    Serial.print(" E="); Serial.print(error);
    Serial.print(" PWM_L="); Serial.print(pwmLeft);
    Serial.print(" PWM_R="); Serial.println(pwmRight);

    delay(20);
  }

  opresteMotoarele();

  long deltaLeft = pulsesLeft;
  long deltaRight = pulsesRight;
  updateOdometrie(deltaLeft, deltaRight);
}

void rotireLaUnghi(float angle_deg) {
  float arc_len = (abs(angle_deg) / 360.0) * ROBOT_WIDTH_CM * 3.1416;
  long ticks = arc_len * TICKS_PER_CM * correction;
  resetPulses();

  if (angle_deg > 0) {
    digitalWrite(motorLeftDIR, HIGH);
    digitalWrite(motorRightDIR, LOW);
  } else {
    digitalWrite(motorLeftDIR, LOW);
    digitalWrite(motorRightDIR, HIGH);
  }

  analogWrite(motorLeftPWM, PWM_SPEED);
  analogWrite(motorRightPWM, PWM_SPEED);

  while ((pulsesLeft + pulsesRight) / 2 < ticks) {
    Serial.print("rot="); Serial.println((pulsesLeft + pulsesRight) / 2);
  }

  opresteMotoarele();

  long deltaLeft = pulsesLeft;
  long deltaRight = pulsesRight;
  updateOdometrie(deltaLeft, deltaRight);
}

void updateOdometrie(long deltaLeft, long deltaRight) {
  float distLeft = deltaLeft / TICKS_PER_CM;
  float distRight = deltaRight / TICKS_PER_CM;

  float delta_s = (distLeft + distRight) / 2.0;
  float delta_theta = (distRight - distLeft) / ROBOT_WIDTH_CM;

  x += delta_s * cos(theta + delta_theta / 2.0);
  y += delta_s * sin(theta + delta_theta / 2.0);
  theta += delta_theta;

  Serial.print("ODOMETRIE → x = ");
  Serial.print(x); Serial.print(" cm, y = ");
  Serial.print(y); Serial.print(" cm, θ = ");
  Serial.println(theta * 180.0 / 3.1416);  // conversie la grade
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