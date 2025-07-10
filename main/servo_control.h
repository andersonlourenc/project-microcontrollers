#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

#define SERVO_X_PIN 4
#define SERVO_Y_PIN 25

Servo servoX;
Servo servoY;

int angleX = 90;
int angleY = 90;

bool servosAtivos = false;

void setupServos() {
  servoX.setPeriodHertz(50);
  servoY.setPeriodHertz(50);
  servoX.attach(SERVO_X_PIN, 500, 2400);
  servoY.attach(SERVO_Y_PIN, 500, 2400);
  servoX.write(angleX);
  servoY.write(angleY);
  Serial.println("Servos inicializados - X:" + String(angleX) + " Y:" + String(angleY));
}

void ligaServos() {
  if (!servosAtivos) {
    servoX.attach(SERVO_X_PIN, 500, 2400);
    servoY.attach(SERVO_Y_PIN, 500, 2400);
    servoX.write(angleX);
    servoY.write(angleY);
    servosAtivos = true;
  }
}

void desligaServos() {
  if (servosAtivos) {
    servoX.detach();
    servoY.detach();
    servosAtivos = false;
  }
}

void setServoX(int angle) {
  angleX = constrain(angle, 0, 180);
  Serial.println("Servo X -> " + String(angleX));
  servoX.write(angleX);
}

void setServoY(int angle) {
  angleY = constrain(angle, 30, 120);
  Serial.println("Servo Y -> " + String(angleY));
  servoY.write(angleY);
}

void handleServoCommand(String cmd) {
  if (cmd == "servo_left")  setServoX(angleX - 5);
  if (cmd == "servo_right") setServoX(angleX + 5);
  if (cmd == "servo_up")    setServoY(angleY - 5);
  if (cmd == "servo_down")  setServoY(angleY + 5);
}

#endif
