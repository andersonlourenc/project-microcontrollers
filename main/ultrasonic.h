
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define TRIG_PIN 14
#define ECHO_PIN 12

// Segundo sensor ultrassônico
#define TRIG_PIN_2 34
#define ECHO_PIN_2 35

void setupUltrasonic() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // Setup do segundo sensor
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
}

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

long readDistanceCM2() {
  digitalWrite(TRIG_PIN_2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN_2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN_2, LOW);

  long duration = pulseIn(ECHO_PIN_2, HIGH, 20000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

#endif