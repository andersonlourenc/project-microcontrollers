#ifndef HTML_PAGE_H
#define HTML_PAGE_H

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='pt-br'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no, maximum-scale=1.0, minimum-scale=1.0'>
  <title>Car Control</title>
  <style>
    html, body, .container, .cam-box, .status-display, .dpad, .servo-controls, .btn-row-center {
      -webkit-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
    }
    html, body {
      background: #222;
      color: #fff;
      font-family: 'Segoe UI', Arial, sans-serif;
      margin: 0;
      padding: 0;
      min-height: 100vh;
      height: 100vh;
      overflow: hidden;
      box-sizing: border-box;
      display: flex;
      justify-content: center;
      align-items: flex-start;
    }
    .container {
      width: 100%;
      max-width: 380px;
      margin: 2px auto 0 auto;
      background: #181818;
      border-radius: 12px;
      box-shadow: 0 2px 10px #0006;
      padding: 6px 4px 8px 4px;
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: unset;
    }
    h1 {
      font-size: 1.15rem;
      margin-bottom: 7px;
      font-weight: 400;
      letter-spacing: 1px;
    }
    .cam-box {
      width: 320px;
      height: 180px;
      max-width: 98vw;
      background: #b8adae;
      border-radius: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
      position: relative;
      margin-bottom: 4px;
    }
    .cam-label {
      display: none;
    }
    .cam-btn {
      position: absolute;
      bottom: 10px;
      right: 10px;
      background: #fff;
      border: none;
      border-radius: 50%;
      width: 32px;
      height: 32px;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 1px 4px #0005;
    }
    .cam-btn:active {
      background: #eee;
    }
    .cam-btn::before {
      content: '\1F4F7';
      font-size: 1.3rem;
    }
    .status-display {
      width: 98vw;
      max-width: 340px;
      min-height: 24px;
      background: #111;
      border-radius: 7px;
      margin-bottom: 4px;
      margin-top: 2px;
      padding: 3px 7px;
      font-size: 0.92rem;
      color: #6cf;
      display: flex;
      flex-direction: column;
      justify-content: center;
      box-sizing: border-box;
      border: 1px solid #333;
    }
    .status-line {
      height: 13px;
      overflow: hidden;
      white-space: nowrap;
      text-overflow: ellipsis;
    }
    .dpad {
      display: grid;
      grid-template-columns: 36px 36px 36px;
      grid-template-rows: 36px 36px 36px;
      gap: 3px;
      margin: 6px 0 6px 0;
    }
    .dpad-btn {
      background: #eee;
      border: none;
      border-radius: 50%;
      width: 36px;
      height: 36px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1.15rem;
      color: #222;
      cursor: pointer;
      box-shadow: 0 1px 4px #0005;
      transition: background 0.15s;
    }
    .dpad-btn:active {
      background: #b8adae;
    }
    .servo-controls {
      margin-top: 4px;
      width: 100%;
      background: #222;
      border-radius: 7px;
      padding: 4px 3px 4px 3px;
      box-sizing: border-box;
      margin-bottom: 2px;
    }
    .servo-label {
      font-size: 0.92rem;
      margin-bottom: 2px;
      color: #fff;
    }
    .slider-row {
      display: flex;
      align-items: center;
      margin-bottom: 3px;
    }
    .slider-row:last-child {
      margin-bottom: 0;
    }
    .slider-title {
      width: 38px;
      font-size: 0.85rem;
      color: #ccc;
    }
    .slider-value {
      width: 22px;
      text-align: right;
      margin-left: 3px;
      color: #fff;
      font-size: 0.85rem;
    }
    .servo-slider {
      flex: 1;
      margin: 0 3px;
      accent-color: #3a2fff;
      height: 13px;
    }
    .btn-row-center {
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 4px;
      margin-top: 4px;
    }
    .reset-btn {
      width: auto;
      min-width: 55px;
      padding: 5px 7px;
      background: #3a2fff;
      color: #fff;
      border: none;
      border-radius: 6px;
      font-size: 0.92rem;
      font-weight: 500;
      cursor: pointer;
      transition: background 0.15s;
      box-shadow: 0 1px 4px #0005;
      margin-top: 0;
    }
    .reset-btn:active {
      background: #2a1fa0;
    }
    .small-btn {
      width: auto;
      min-width: 55px;
      padding: 5px 7px;
      font-size: 0.92rem;
      margin-top: 0;
    }
    .led-slider-row {
      position: absolute;
      top: 8px;
      left: 12px;
      right: 50px;
      display: flex;
      align-items: center;
      z-index: 2;
    }
    .led-slider {
      flex: 1;
      accent-color: #ffb300;
      height: 10px;
    }
    .led-value {
      width: 22px;
      color: #ffb300;
      font-size: 0.85rem;
      margin-left: 4px;
      text-align: right;
    }
    .mode-row {
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 12px;
      margin-bottom: 2px;
      margin-top: 0;
    }
    .mode-btn {
      background: #444;
      color: #fff;
      border: none;
      border-radius: 6px;
      padding: 5px 10px;
      font-size: 0.92rem;
      font-weight: 500;
      cursor: pointer;
      transition: background 0.15s;
      box-shadow: 0 1px 3px #0003;
    }
    .mode-btn.active {
      background: #3a2fff;
      color: #fff;
    }
    .mode-label {
      font-size: 0.95rem;
      color: #3a2fff;
      font-weight: 600;
      letter-spacing: 1px;
    }
    .speed-control {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-bottom: 8px;
      gap: 4px;
    }
    .speed-label {
      font-size: 0.9rem;
      color: #ffb300;
      font-weight: 500;
    }
    .speed-slider {
      width: 200px;
      accent-color: #ffb300;
      height: 8px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Car Control</h1>
    <div class="cam-box">
      <div class="led-slider-row">
        <input type="range" min="0" max="255" value="0" class="led-slider" id="ledSlider" oninput="setLedIntensity()" aria-label="Intensidade do flash" tabindex="0">
        <span class="led-value" id="ledVal">0</span>
      </div>
      <img id="cam" src="http://172.20.10.2:81/stream" style="width:100%;height:100%;object-fit:cover;border-radius:12px;" alt="ESP CAM" aria-label="Vídeo da câmera" tabindex="0">
      <button class="cam-btn" title="Tirar foto" aria-label="Tirar foto" tabindex="0" onclick="takePhoto()"></button>
    </div>
    <div class="status-display">
      <div class="status-line" id="status1">&nbsp;</div>
      <div class="status-line" id="status2">&nbsp;</div>
      <div class="status-line" id="sensorVals">&nbsp;</div>
    </div>
    <div class="mode-row">
      <button id="btnAuto" class="mode-btn active" onclick="setMode(true)" aria-label="Modo autônomo" tabindex="0">Autônomo</button>
      <button id="btnManual" class="mode-btn" onclick="setMode(false)" aria-label="Modo manual com anticolisão" tabindex="0">Manual + Anticolisão</button>
      <span class="mode-label" id="modeLabel">Modo: Autônomo</span>
    </div>
    <div class="speed-control">
      <div class="speed-label">Velocidade: <span id="speedValue">255</span></div>
      <input type="range" min="50" max="255" value="255" class="speed-slider" id="speedSlider" oninput="setSpeed()" aria-label="Controle de velocidade" tabindex="0">
    </div>
    <div class="dpad">
      <div></div>
      <button class="dpad-btn" onmousedown="startCommand('forward')" onmouseup="stopCommand()" ontouchstart="startCommand('forward')" ontouchend="stopCommand()" title="Frente" aria-label="Mover para frente" tabindex="0">&#9650;</button>
      <div></div>
      <button class="dpad-btn" onmousedown="startCommand('left')" onmouseup="stopCommand()" ontouchstart="startCommand('left')" ontouchend="stopCommand()" title="Esquerda" aria-label="Mover para esquerda" tabindex="0">&#9664;</button>
      <button class="dpad-btn" onclick="sendCmd('stop')" title="Parar" aria-label="Parar" tabindex="0">&#9679;</button>
      <button class="dpad-btn" onmousedown="startCommand('right')" onmouseup="stopCommand()" ontouchstart="startCommand('right')" ontouchend="stopCommand()" title="Direita" aria-label="Mover para direita" tabindex="0">&#9654;</button>
      <div></div>
      <button class="dpad-btn" onmousedown="startCommand('reverse')" onmouseup="stopCommand()" ontouchstart="startCommand('reverse')" ontouchend="stopCommand()" title="Ré" aria-label="Mover para trás" tabindex="0">&#9660;</button>
      <div></div>
    </div>
    <div class="servo-controls">
      <div class="servo-label">Controle servos</div>
      <div class="slider-row">
        <span class="slider-title">eixo: X</span>
        <input type="range" min="0" max="180" value="90" class="servo-slider" id="servoX" oninput="updateServo('X')" aria-label="Servo eixo X" tabindex="0">
        <span class="slider-value" id="servoXVal">90°</span>
      </div>
      <div class="slider-row">
        <span class="slider-title">eixo: Y</span>
        <input type="range" min="30" max="120" value="90" class="servo-slider" id="servoY" oninput="updateServo('Y')" aria-label="Servo eixo Y" tabindex="0">
        <span class="slider-value" id="servoYVal">90°</span>
      </div>
      <div class="btn-row-center">
        <button class="reset-btn small-btn" onclick="resetServos()" aria-label="Resetar servos" tabindex="0">Resetar</button>
        <button id="servosToggleBtn" class="reset-btn small-btn" style="background:#2ecc40;" onclick="toggleServos()" aria-label="Ligar ou desligar servos" tabindex="0">Ligar servos</button>
      </div>
    </div>
  </div>
  <script>
    let currentCommand = 'stop';
    let commandInterval = null;
    
    function sendCmd(cmd) {
      currentCommand = cmd;
      fetch('/' + cmd, {method: 'GET'});
    }
    
    function startCommand(cmd) {
      if (currentCommand !== cmd) {
        currentCommand = cmd;
        fetch('/' + cmd, {method: 'GET'});
      }
    }
    
    function stopCommand() {
      if (currentCommand !== 'stop') {
        currentCommand = 'stop';
        fetch('/stop', {method: 'GET'});
      }
    }
    
    // Garantir que o comando pare quando o mouse sair do botão
    document.addEventListener('mouseup', function() {
      stopCommand();
    });
    
    document.addEventListener('touchend', function() {
      stopCommand();
    });
    function updateServo(axis) {
      let val = document.getElementById('servo' + axis).value;
      document.getElementById('servo' + axis + 'Val').innerText = val + '°';
      fetch('/servo' + axis + '?value=' + val, {method: 'GET'});
    }
    function resetServos() {
      fetch('/servo_reset', {method: 'GET'}).then(() => {
        document.getElementById('servoX').value = 90;
        document.getElementById('servoY').value = 70;
        document.getElementById('servoXVal').innerText = '90°';
        document.getElementById('servoYVal').innerText = '70°';
      });
    }
    function updateServosToggleBtn(on) {
      const btn = document.getElementById('servosToggleBtn');
      if (on) {
        btn.innerText = 'Desligar servos';
        btn.style.background = '#ff4136';
      } else {
        btn.innerText = 'Ligar servos';
        btn.style.background = '#2ecc40';
      }
    }
    function toggleServos() {
      fetch('/servos_toggle', {method: 'GET'})
        .then(r => r.json())
        .then(obj => updateServosToggleBtn(obj.on));
    }
    let modoAutonomo = true;
    function setMode(auto) {
      modoAutonomo = auto;
      document.getElementById('btnAuto').classList.toggle('active', auto);
      document.getElementById('btnManual').classList.toggle('active', !auto);
      document.getElementById('modeLabel').innerText = auto ? 'Modo: Autônomo' : 'Modo: Manual + Anticolisão';
      fetch('/set_mode?auto=' + (auto ? '1' : '0'));
    }
    
    function setSpeed() {
      let speed = document.getElementById('speedSlider').value;
      document.getElementById('speedValue').innerText = speed;
      fetch('/set_speed?speed=' + speed);
    }
    function setLedIntensity() {
      let val = document.getElementById('ledSlider').value;
      document.getElementById('ledVal').innerText = val;
      fetch('http://172.20.10.2/control?var=led_intensity&val=' + val);
    }
    function takePhoto() {
      window.open('http://172.20.10.2/capture', '_blank');
    }
    // Ao carregar a página, consulta o estado dos servos
    window.addEventListener('DOMContentLoaded', () => {
      fetch('/servos_status').then(r => r.json()).then(obj => updateServosToggleBtn(obj.on));
      fetch('/get_mode').then(r => r.json()).then(obj => setMode(obj.auto));
    });
    // Atualiza o display de status a cada 1s
    setInterval(() => {
      fetch('/status').then(r => r.json()).then(obj => {
        document.getElementById('status1').innerText = obj.line1 || '';
        document.getElementById('status2').innerText = obj.line2 || '';
        document.getElementById('sensorVals').innerText =
          'US1: ' + (obj.d1 >= 0 ? obj.d1 + 'cm' : '--') + ' | US2: ' + (obj.d2 >= 0 ? obj.d2 + 'cm' : '--');
      }).catch(() => {
        document.getElementById('status1').innerText = 'Sem conexão';
        document.getElementById('status2').innerText = '';
        document.getElementById('sensorVals').innerText = '';
      });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";

#endif
