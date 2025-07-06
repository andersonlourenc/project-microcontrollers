#include <WiFi.h>
#include <WebServer.h>
#include "secrects.h"
#include "motor_control.h"
#include "ultrasonic.h"
#include "html_interface.h"


WebServer server(80);

void setup() {
  Serial.begin(115200);

  setupMotors();
  setupUltrasonic();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\WiFi connected! IP: " + WiFi.localIP().toString());

  server.on("/", []() { server.send(200, "text/html", getHTML()); });

  server.on("/forward", []() {
    if (readDistanceCM() > stopDistance) moveForward();
    else stopMotors();
    server.send(204);
  });

  server.on("/reverse", []() { moveReverse(); server.send(204); });
  server.on("/left", []() {moveLeft(); server.send(204); });
  server.on("right", []() {moveRight(); server.send(204); });
  server.on("/stop", []() { stopMotors(); server.send(204); });

  server.begin();

}

  void loop() {

    long distance = readDistanceCM();

    if (distance > 0 && distance < 15) {
      stopMotors();
      Serial.println("Obstáculo detectado! Virando à direita...");
      delay(300);
      moveRight();
      delay(500);
      stopMotors();
    }
    server.handleClient();
  }

