#include <WiFi.h>
#include <WebServer.h>
#include <motor_control.h>

// Motor pins
#define IN1  27
#define IN2  26
#define IN3  33
#define IN4  32

// Wi-Fi credentials
const char* ssid = "brisa-2238301";
const char* password = "favbk07p";

// Web server on port 80
WebServer server(80);

// Movement functions
void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveReverse() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void moveLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

// HTML interface
String getHTML() {
  return R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>Car Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { font-family: sans-serif; text-align: center; margin-top: 30px; }
      .btn {
        width: 100px; height: 100px;
        font-size: 18px;
        margin: 10px;
        border-radius: 10px;
      }
    </style>
  </head>
  <body>
    <h2>Car Control</h2>
    <div>
      <button class="btn" onmousedown="move('forward')" onmouseup="stop()" ontouchstart="move('forward')" ontouchend="stop()">FORWARD</button><br>
      <button class="btn" onmousedown="move('left')" onmouseup="stop()" ontouchstart="move('left')" ontouchend="stop()">LEFT</button>
      <button class="btn" onmousedown="move('right')" onmouseup="stop()" ontouchstart="move('right')" ontouchend="stop()">RIGHT</button>
      <button class="btn" onmousedown="move('reverse')" onmouseup="stop()" ontouchstart="move('reverse')" ontouchend="stop()">REVERSE</button>
    </div>

    <script>
      function move(dir) {
        fetch('/' + dir);
      }
      function stop() {
        fetch('/stop');
      }
    </script>
  </body>
  </html>
  )rawliteral";
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP address: " + WiFi.localIP().toString());

  // Routes
  server.on("/", []() { server.send(200, "text/html", getHTML()); });
  server.on("/forward", []() { moveForward(); server.send(204); });
  server.on("/reverse", []() { moveReverse(); server.send(204); });
  server.on("/left", []() { moveLeft(); server.send(204); });
  server.on("/right", []() { moveRight(); server.send(204); });
  server.on("/stop", []() { stopMotors(); server.send(204); });

  server.begin();
}

void loop() {
  server.handleClient();
}
