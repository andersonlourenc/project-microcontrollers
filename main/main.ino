#define ESP32_PWM_CHANNEL_OFFSET 16

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <ESP32Servo.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "secrets.h"
#include "motor_control.h"
#include "ultrasonic.h"
#include "servo_control.h"
#include "html_page.h"

WebServer server(80);


#define MIN_DIST_CM    20
#define SLOW_DIST_CM   50
#define MAX_SPEED     255
#define MIN_SPEED     100
#define TURN_TIME     600


String currentCommand = "stop";
bool deviating = false;
unsigned long tDetour = 0;
unsigned long lastSense = 0;


long lastD1 = -1;
long lastD2 = -1;


bool modoAutonomo = false;


String statusLine1 = "";
String statusLine2 = "";


void updateStatus(String msg) {
  statusLine2 = statusLine1;
  statusLine1 = msg;
  Serial.println(msg);
}


QueueHandle_t commandQueue;
QueueHandle_t distanceQueue;


TaskHandle_t webServerTaskHandle = NULL;
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t motorControlTaskHandle = NULL;


void webServerTask(void *parameter) {
  while (true) {
    server.handleClient();
    ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
  }
}


void sensorTask(void *parameter) {
  while (true) {
    long d1 = readDistanceCM();
    long d2 = readDistanceCM2();
    lastD1 = d1;
    lastD2 = d2;
    long distance = d1;
    if (d2 > 0 && (d1 < 0 || d2 < d1)) distance = d2; 

    if (distance > 0) {
      xQueueSend(distanceQueue, &distance, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(120));
  }
}

// Task de controle de motores
void motorControlTask(void *parameter) {
  long distance;
  bool desviando = false;
  unsigned long tempoDesvio = 0;
  int ultimaDirecao = 1; 

  while (true) {
    if (xQueueReceive(distanceQueue, &distance, pdMS_TO_TICKS(10)) == pdTRUE) {
      if (modoAutonomo) {
        bool obstaculo1 = (lastD1 > 0 && lastD1 < MIN_DIST_CM);
        bool obstaculo2 = (lastD2 > 0 && lastD2 < MIN_DIST_CM);
        if (obstaculo1 && obstaculo2) {
          // Ambos sensores bloqueados: para e tenta virar alternando
          stopMotors();
          setMotorSpeed(MAX_SPEED, MAX_SPEED);
          if (ultimaDirecao == 1) {
            moveRight();
            updateStatus("Obstáculo nos dois lados! Tentando direita...");
            ultimaDirecao = -1;
          } else {
            moveLeft();
            updateStatus("Obstáculo nos dois lados! Tentando esquerda...");
            ultimaDirecao = 1;
          }
          desviando = true;
          tempoDesvio = millis();
        } else if (obstaculo1) {
          stopMotors();
          setMotorSpeed(MAX_SPEED, MAX_SPEED);
          moveRight();
          updateStatus("Obstáculo à esquerda! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
          ultimaDirecao = 1;
        } else if (obstaculo2) {
          stopMotors();
          setMotorSpeed(MAX_SPEED, MAX_SPEED);
          moveLeft();
          updateStatus("Obstáculo à direita! Virando à esquerda...");
          desviando = true;
          tempoDesvio = millis();
          ultimaDirecao = -1;
        } else if (distance < SLOW_DIST_CM) {
          uint8_t speed = map(distance, MIN_DIST_CM, SLOW_DIST_CM, MIN_SPEED, MAX_SPEED);
          setMotorSpeed(speed, speed);
          moveForward();
          updateStatus("Reduzindo velocidade: " + String(speed));
          desviando = false;
        } else {
          setMotorSpeed(MAX_SPEED, MAX_SPEED);
          moveForward();
          updateStatus("Caminho livre! Velocidade máxima.");
          desviando = false;
        }
        if (currentCommand == "stop" && (obstaculo1 || obstaculo2)) {
          stopMotors();
          setMotorSpeed(MAX_SPEED, MAX_SPEED);
          if (obstaculo1 && !obstaculo2) moveRight();
          else if (obstaculo2 && !obstaculo1) moveLeft();
          else if (obstaculo1 && obstaculo2) {
            if (ultimaDirecao == 1) moveRight();
            else moveLeft();
          }
          updateStatus("Parado, mas obstáculo! Desviando...");
          desviando = true;
          tempoDesvio = millis();
        }
        // Não executa comandos manuais no modo autônomo
      } else {
        // Modo manual com anticolisão aprimorado
        bool obstaculo1 = (lastD1 > 0 && lastD1 < MIN_DIST_CM);
        bool obstaculo2 = (lastD2 > 0 && lastD2 < MIN_DIST_CM);
        if (currentCommand == "forward" && (obstaculo1 || obstaculo2)) {
          stopMotors();
          updateStatus("Obstáculo à frente! Parando para evitar colisão.");
        } else if (currentCommand == "reverse" && (obstaculo1 || obstaculo2)) {
          stopMotors();
          updateStatus("Obstáculo atrás! Parando para evitar colisão.");
        } else if (distance < SLOW_DIST_CM && currentCommand == "forward") {
          uint8_t speed = map(distance, MIN_DIST_CM, SLOW_DIST_CM, MIN_SPEED, MAX_SPEED);
          setMotorSpeed(speed, speed);
          moveForward();
          updateStatus("Reduzindo velocidade: " + String(speed));
        } else {
          // Só se move enquanto o comando está ativo
          if (currentCommand == "forward") {
            setMotorSpeed(MAX_SPEED, MAX_SPEED);
            moveForward();
          } else if (currentCommand == "reverse") {
            setMotorSpeed(MAX_SPEED, MAX_SPEED);
            moveReverse();
          } else if (currentCommand == "left") {
            setMotorSpeed(MAX_SPEED, MAX_SPEED);
            moveLeft();
          } else if (currentCommand == "right") {
            setMotorSpeed(MAX_SPEED, MAX_SPEED);
            moveRight();
          } else {
            stopMotors();
          }
        }
      }
    }
    if (desviando && millis() - tempoDesvio > TURN_TIME) {
      stopMotors();
      desviando = false;
      updateStatus("Desvio finalizado.");
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Ordem CRÍTICA: Servos primeiro!
  setupServos();
  delay(200);
  desligaServos(); // Servos iniciam desligados
  
  setupMotors();
  setupUltrasonic();

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
  updateStatus("WiFi conectado!");

  // OTA
  ArduinoOTA.setHostname("esp32-robot");
  ArduinoOTA.begin();

  // Rotas principais
  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/forward", []() {
    currentCommand = "forward";
    updateStatus("Comando: frente");
    server.send(204);
  });

  server.on("/reverse", []() {
    currentCommand = "reverse";
    updateStatus("Comando: ré");
    server.send(204);
  });

  server.on("/left", []() {
    currentCommand = "left";
    updateStatus("Comando: esquerda");
    server.send(204);
  });

  server.on("/right", []() {
    currentCommand = "right";
    updateStatus("Comando: direita");
    server.send(204);
  });

  server.on("/stop", []() {
    currentCommand = "stop";
    updateStatus("Comando: parar");
    server.send(204);
  });

  server.on("/servoX", []() {
    if (server.hasArg("value")) {
      setServoX(server.arg("value").toInt());
      updateStatus("Servo X: " + server.arg("value"));
    }
    server.send(204);
  });

  server.on("/servoY", []() {
    if (server.hasArg("value")) {
      setServoY(server.arg("value").toInt());
      updateStatus("Servo Y: " + server.arg("value"));
    }
    server.send(204);
  });

  server.on("/servo_reset", []() {
    setServoX(90);
    setServoY(90);
    updateStatus("Servos resetados para origem");
    server.send(204);
  });

  server.on("/servos_toggle", []() {
    if (servosAtivos) {
      desligaServos();
      updateStatus("Servos desligados");
    } else {
      ligaServos();
      updateStatus("Servos ligados");
    }
    String json = String("{\"on\":") + (servosAtivos ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });
  server.on("/servos_status", []() {
    String json = String("{\"on\":") + (servosAtivos ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  server.on("/status", []() {
    String json = "{";
    json += "\"line1\":\"" + statusLine1 + "\",";
    json += "\"line2\":\"" + statusLine2 + "\",";
    json += "\"d1\":" + String(lastD1) + ",";
    json += "\"d2\":" + String(lastD2) + "}";
    server.send(200, "application/json", json);
  });

  server.on("/set_mode", []() {
    if (server.hasArg("auto")) {
      modoAutonomo = (server.arg("auto") == "1");
      updateStatus(modoAutonomo ? "Modo: Autônomo" : "Modo: Manual + Anticolisão");
    }
    server.send(204);
  });
  server.on("/get_mode", []() {
    String json = String("{\"auto\":") + (modoAutonomo ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  

  setServoX(90);
  setServoY(90);
  

  distanceQueue = xQueueCreate(5, sizeof(long));
  
  // Criação das tasks
  xTaskCreatePinnedToCore(
    webServerTask,        // Função da task
    "WebServer",          // Nome da task
    10000,                // Stack size
    NULL,                 // Parâmetros
    1,                    // Prioridade
    &webServerTaskHandle, // Handle da task
    0                     // Core (0 ou 1)
  );
  
  xTaskCreatePinnedToCore(
    sensorTask,           // Função da task
    "Sensor",             // Nome da task
    4096,                 // Stack size
    NULL,                 // Parâmetros
    2,                    // Prioridade
    &sensorTaskHandle,    // Handle da task
    1                     // Core (0 ou 1)
  );
  
  xTaskCreatePinnedToCore(
    motorControlTask,    
    "MotorControl",      
    4096,                
    NULL,                
    2,                   
    &motorControlTaskHandle, 
    1                     
  );
  
  Serial.println("Setup completo com threads");
}

void loop() {

  vTaskDelay(pdMS_TO_TICKS(1000));
} 