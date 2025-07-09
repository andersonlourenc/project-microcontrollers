#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

#define SERVO_X_PIN 4
#define SERVO_Y_PIN 13

Servo servoX;
Servo servoY;

int angleX = 0;
int angleY = 90;

void setupServos() {
  servoX.setPeriodHertz(50);
  servoY.setPeriodHertz(50);
  servoX.attach(SERVO_X_PIN, 500, 2400);
  servoY.attach(SERVO_Y_PIN, 500, 2400);
  servoX.write(angleX);
  servoY.write(angleY);
}

void setServoX(int angle) {
  angleX = constrain(angle, 0, 180);
  servoX.write(angleX);
}

void setServoY(int angle) {
  angleY = constrain(angle, 0, 110);
  servoY.write(angleY);
}

void handleServoCommand(String cmd) {
  if (cmd == "servo_left")  setServoX(angleX - 5);
  if (cmd == "servo_right") setServoX(angleX + 5);
  if (cmd == "servo_up")    setServoY(angleY - 5);
  if (cmd == "servo_down")  setServoY(angleY + 5);
}

#endif
