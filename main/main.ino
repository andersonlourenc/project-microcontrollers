#define ESP32_PWM_CHANNEL_OFFSET 16

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <ESP32Servo.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <HTTPClient.h>

#include "secrets.h"
#include "motor_control.h"
#include "ultrasonic.h"
#include "servo_control.h"
#include "html_page.h"

WebServer server(80);


#define MIN_DIST_CM    15
#define SLOW_DIST_CM   40
#define CRITICAL_DIST  10
#define MAX_SPEED     255
#define MIN_SPEED     150
#define TURN_SPEED    255
#define CURVE_SPEED   255
#define TURN_TIME     800

// Controle de velocidade
uint8_t currentSpeed = MAX_SPEED;
uint8_t targetSpeed = MAX_SPEED;
bool speedRamping = false;


String currentCommand = "stop";
bool deviating = false;
unsigned long tDetour = 0;
unsigned long lastSense = 0;


long lastD1 = -1;
long lastD2 = -1;


bool modoAutonomo = false;


String statusLine1 = "";
String statusLine2 = "";
String cameraIP = "172.20.10.2"; // IP da ESP32-CAM


void updateStatus(String msg) {
  statusLine2 = statusLine1;
  statusLine1 = msg;
  Serial.println(msg);
}

// Função para controle suave de velocidade
void setTargetSpeed(uint8_t speed) {
  targetSpeed = speed;
  speedRamping = true;
}

// Função para aplicar rampa de velocidade
void applySpeedRamp() {
  if (speedRamping) {
    if (currentSpeed < targetSpeed) {
      currentSpeed = min((uint8_t)(currentSpeed + 5), targetSpeed);
    } else if (currentSpeed > targetSpeed) {
      currentSpeed = max((uint8_t)(currentSpeed - 5), targetSpeed);
    } else {
      speedRamping = false;
    }
  }
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

    // Debug dos sensores a cada 2 segundos
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 2000) {
      Serial.print("Sensor 1 (GPIO 12): ");
      if (d1 > 0) Serial.print(d1);
      else Serial.print("ERRO");
      Serial.print("cm | Sensor 2 (GPIO 13): ");
      if (d2 > 0) Serial.print(d2);
      else Serial.print("ERRO");
      Serial.println("cm");
      lastDebug = millis();
    }

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
    // Aplicar rampa de velocidade
    applySpeedRamp();
    
    if (xQueueReceive(distanceQueue, &distance, pdMS_TO_TICKS(10)) == pdTRUE) {
      if (modoAutonomo) {
        bool obstaculo1 = (lastD1 > 0 && lastD1 < MIN_DIST_CM);
        bool obstaculo2 = (lastD2 > 0 && lastD2 < MIN_DIST_CM);
        bool obstaculoCritico1 = (lastD1 > 0 && lastD1 < CRITICAL_DIST);
        bool obstaculoCritico2 = (lastD2 > 0 && lastD2 < CRITICAL_DIST);
        
        if (obstaculoCritico1 || obstaculoCritico2) {
          // Obstáculo crítico: para e vira para direita
          stopMotors();
          setTargetSpeed(TURN_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveRight();
          updateStatus("Obstáculo crítico! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
          ultimaDirecao = 1;
        } else if (obstaculo1 && obstaculo2) {
          // Ambos sensores bloqueados: vira para direita sem parar
          setTargetSpeed(TURN_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveRight();
          updateStatus("Obstáculo nos dois lados! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
          ultimaDirecao = 1;
        } else if (obstaculo1) {
          // Obstáculo à esquerda: vira para direita sem parar
          setTargetSpeed(TURN_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveRight();
          updateStatus("Obstáculo à esquerda! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
          ultimaDirecao = 1;
        } else if (obstaculo2) {
          // Obstáculo à direita: vira para direita (preferência)
          setTargetSpeed(TURN_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveRight();
          updateStatus("Obstáculo à direita! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
          ultimaDirecao = 1;
        } else if (distance < SLOW_DIST_CM) {
          // Reduz velocidade apenas próximo aos obstáculos
          uint8_t speed = map(distance, MIN_DIST_CM, SLOW_DIST_CM, MIN_SPEED, MAX_SPEED);
          setTargetSpeed(speed);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveForward();
          updateStatus("Reduzindo velocidade: " + String(currentSpeed));
          desviando = false;
        } else {
          // Caminho livre: velocidade máxima
          setTargetSpeed(MAX_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveForward();
          updateStatus("Caminho livre! Velocidade máxima: " + String(currentSpeed));
          desviando = false;
        }
      } else {
        // Modo manual com anticolisão aprimorado
        bool obstaculo1 = (lastD1 > 0 && lastD1 < MIN_DIST_CM);
        bool obstaculo2 = (lastD2 > 0 && lastD2 < MIN_DIST_CM);
        bool obstaculoCritico1 = (lastD1 > 0 && lastD1 < CRITICAL_DIST);
        bool obstaculoCritico2 = (lastD2 > 0 && lastD2 < CRITICAL_DIST);
        
        // PRIORIDADE 1: Verificar obstáculos críticos (sobrescreve TUDO)
        if (obstaculoCritico1 || obstaculoCritico2) {
          stopMotors();
          setTargetSpeed(TURN_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveRight();
          updateStatus("Obstáculo crítico! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
        }
        // PRIORIDADE 2: Verificar obstáculos normais (sobrescreve comandos)
        else if (obstaculo1 || obstaculo2) {
          setTargetSpeed(TURN_SPEED);
          setMotorSpeed(currentSpeed, currentSpeed);
          moveRight();
          updateStatus("Obstáculo detectado! Virando à direita...");
          desviando = true;
          tempoDesvio = millis();
        }
        // PRIORIDADE 3: Executar comando atual (apenas se não há obstáculos)
        else {
          if (currentCommand == "forward") {
            if (distance < SLOW_DIST_CM) {
              uint8_t speed = map(distance, MIN_DIST_CM, SLOW_DIST_CM, MIN_SPEED, MAX_SPEED);
              setTargetSpeed(speed);
              setMotorSpeed(currentSpeed, currentSpeed);
              moveForward();
              updateStatus("Reduzindo velocidade: " + String(currentSpeed));
            } else {
              setTargetSpeed(MAX_SPEED);
              setMotorSpeed(currentSpeed, currentSpeed);
              moveForward();
              updateStatus("Movendo para frente - Velocidade máxima: " + String(currentSpeed));
            }
          } else if (currentCommand == "reverse") {
            setTargetSpeed(MAX_SPEED);
            setMotorSpeed(currentSpeed, currentSpeed);
            moveReverse();
            updateStatus("Movendo para trás - Velocidade máxima: " + String(currentSpeed));
          } else if (currentCommand == "left") {
            setTargetSpeed(CURVE_SPEED);
            setMotorSpeed(currentSpeed, currentSpeed);
            moveLeft();
            updateStatus("Virando à esquerda - Velocidade máxima: " + String(currentSpeed));
          } else if (currentCommand == "right") {
            setTargetSpeed(CURVE_SPEED);
            setMotorSpeed(currentSpeed, currentSpeed);
            moveRight();
            updateStatus("Virando à direita - Velocidade máxima: " + String(currentSpeed));
          } else if (currentCommand == "stop") {
            stopMotors();
            updateStatus("Parado");
          } else {
            stopMotors();
            updateStatus("Parado");
          }
        }
      }
    }
    
    // Finalizar desvio após tempo determinado
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
  
  // Teste inicial dos sensores
  Serial.println("Testando sensores ultrassônicos...");
  delay(1000);
  
  long test1 = readDistanceCM();
  long test2 = readDistanceCM2();
  
  Serial.print("Sensor 1 (GPIO 12): ");
  if (test1 > 0) Serial.print(test1);
  else Serial.print("ERRO");
  Serial.println("cm");
  
  Serial.print("Sensor 2 (GPIO 13): ");
  if (test2 > 0) Serial.print(test2);
  else Serial.print("ERRO");
  Serial.println("cm");
  
  if (test1 < 0 && test2 < 0) {
    Serial.println("ATENÇÃO: Ambos os sensores estão com erro!");
    Serial.println("Verifique as conexões dos sensores ultrassônicos");
  } else if (test1 < 0) {
    Serial.println("ATENÇÃO: Sensor 1 (GPIO 12) com erro!");
  } else if (test2 < 0) {
    Serial.println("ATENÇÃO: Sensor 2 (GPIO 13) com erro!");
  } else {
    Serial.println("Sensores funcionando corretamente!");
  }

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

  server.on("/foto_telegram", []() {
    tirarFotoEnviarTelegram();
    server.send(200, "text/plain", "Foto enviada para o Telegram!");
  });

  server.on("/set_camera_ip", []() {
    if (server.hasArg("ip")) {
      cameraIP = server.arg("ip");
      updateStatus("IP da câmera configurado: " + cameraIP);
    }
    server.send(204);
  });

  server.on("/test_camera", []() {
    WiFiClient client;
    HTTPClient http;
    
    String url = "http://" + cameraIP + "/";
    Serial.println("Testando conectividade com: " + url);
    
    http.begin(client, url);
    http.setTimeout(3000);
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String response = "Conexão OK! IP: " + cameraIP;
      server.send(200, "text/plain", response);
      updateStatus("Câmera conectada: " + cameraIP);
    } else {
      String response = "Erro de conexão: " + String(httpCode) + " - IP: " + cameraIP;
      server.send(200, "text/plain", response);
      updateStatus("Erro ao conectar com câmera: " + String(httpCode));
    }
    http.end();
  });

  server.on("/set_speed", []() {
    if (server.hasArg("speed")) {
      uint8_t speed = server.arg("speed").toInt();
      if (speed >= 50 && speed <= 255) {
        setTargetSpeed(speed);
        updateStatus("Velocidade ajustada para: " + String(speed));
        server.send(200, "text/plain", "Velocidade: " + String(speed));
      } else {
        server.send(400, "text/plain", "Velocidade deve estar entre 50 e 255");
      }
    } else {
      server.send(400, "text/plain", "Parâmetro 'speed' necessário");
    }
  });

  server.on("/get_speed", []() {
    String json = "{\"current\":" + String(currentSpeed) + ",\"target\":" + String(targetSpeed) + "}";
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

// Dados do Telegram (usados apenas na ESP32-CAM)
// String botToken = "7793189157:AAEJarBHRCMGTUYVsqJCpNw3tnrHYI0T3I0";
// String chatId = "777216271";

void tirarFotoEnviarTelegram() {
  // Esta função será implementada na ESP32-CAM
  // O ESP32 principal apenas aciona a ESP32-CAM via HTTP
  WiFiClient client;
  HTTPClient http;
  
  String url = "http://" + cameraIP + "/foto_telegram";
  Serial.println("Tentando conectar com ESP32-CAM: " + url);
  
  http.begin(client, url);
  http.setTimeout(5000); // 5 segundos de timeout
  
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    updateStatus("Foto enviada para o Telegram!");
  } else {
    updateStatus("Erro ao acionar ESP32-CAM: " + String(httpCode));
    Serial.println("Verifique se a ESP32-CAM está ligada e conectada na mesma rede");
    Serial.println("IP atual da câmera: " + cameraIP);
    Serial.println("Use /set_camera_ip?ip=IP_DA_CAMERA para configurar");
  }
  http.end();
} 