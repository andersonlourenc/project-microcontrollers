#ifndef HTML_PAGE_H
#define HTML_PAGE_H

const char htmlPage[] PROGMEM = R"rawliteral(
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
  <h2>CAR CONTROL</h2>
  <div>
    <button class="btn" onmousedown="move('forward')" onmouseup="stop()">Frente</button><br>
    <button class="btn" onmousedown="move('left')" onmouseup="stop()">Esquerda</button>
    <button class="btn" onmousedown="move('right')" onmouseup="stop()">Direita</button>
    <button class="btn" onmousedown="move('reverse')" onmouseup="stop()">Ré</button>
  </div>

  <h3>Controle da Câmera</h3>
  <button class="btn" onclick="send('servo_up')">Cima</button><br>
  <button class="btn" onclick="send('servo_left')">Esquerda</button>
  <button class="btn" onclick="send('servo_right')">Direita</button><br>
  <button class="btn" onclick="send('servo_down')">Baixo</button>

  <script>
    function move(dir) {
      fetch('/' + dir);
    }
    function stop() {
      fetch('/stop');
    }
    function send(cmd) {
      fetch('/cam?cmd=' + cmd);
    }
  </script>
</body>
</html>
)rawliteral";

#endif
