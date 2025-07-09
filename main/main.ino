#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

#include "secrets.h"
#include "motor_control.h"
#include "ultrasonic.h"
#include "servo_control.h"
#include "html_page.h"

WebServer server(80);

// Constantes de controle
#define MIN_DIST_CM    20
#define SLOW_DIST_CM   50
#define MAX_SPEED     255
#define MIN_SPEED     100
#define TURN_TIME     600

// Variáveis de estado
String currentCommand = "stop";
bool deviating = false;
unsigned long tDetour = 0;
unsigned long lastSense = 0;

void setup() {
  Serial.begin(115200);

  setupMotors();
  setupUltrasonic();
  setupServos();

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  // OTA
  ArduinoOTA.setHostname("esp32-robot");
  ArduinoOTA.begin();

  // Rotas principais
  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/forward", []() {
    currentCommand = "forward";
    server.send(204);
  });

  server.on("/reverse", []() {
    currentCommand = "reverse";
    server.send(204);
  });

  server.on("/left", []() {
    currentCommand = "left";
    server.send(204);
  });

  server.on("/right", []() {
    currentCommand = "right";
    server.send(204);
  });

  server.on("/stop", []() {
    currentCommand = "stop";
    server.send(204);
  });

  // Controle da câmera (servos)
  server.on("/cam", []() {
    if (server.hasArg("cmd")) {
      String cmd = server.arg("cmd");
      handleServoCommand(cmd);
    }
    server.send(204);
  });

  server.begin();
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  // Medição de distância a cada 120ms
  if (millis() - lastSense > 120) {
    lastSense = millis();
    long d = readDistanceCM();

    if (currentCommand == "forward") {
      if (d > 0 && d < SLOW_DIST_CM) {
        if (d < MIN_DIST_CM) {
          stopMotors();
          delay(150);
          setMotorSpeed(MAX_SPEED, MAX_SPEED);
          moveRight();
          deviating = true;
          tDetour = millis();
          Serial.println("Obstáculo detectado! Desviando...");
        } else {
          uint8_t speed = map(d, MIN_DIST_CM, SLOW_DIST_CM, MIN_SPEED, MAX_SPEED);
          setMotorSpeed(speed, speed);
          moveForward();
        }
      } else {
        setMotorSpeed(MAX_SPEED, MAX_SPEED);
        moveForward();
      }
    } else if (!deviating) {
      if (currentCommand == "reverse") moveReverse();
      else if (currentCommand == "left") moveLeft();
      else if (currentCommand == "right") moveRight();
      else stopMotors();
    }
  }

  // Fim do desvio automático
  if (deviating && millis() - tDetour > TURN_TIME) {
    stopMotors();
    deviating = false;
    Serial.println("Desvio finalizado.");
  }
}
