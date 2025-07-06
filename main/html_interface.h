#ifndef HTML_INTERFACE_H
#define HTML_INTERFACE_H

String getHTML() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <title>Car Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
       <style>
      body {
        font-family: sans-serif;
        text-align: center;
        margin-top: 30px;
        user-select: none;         
        -webkit-user-select: none;  
        -ms-user-select: none;    
      }
      .btn {
        width: 100px;
        height: 100px;
        font-size: 18px;
        margin: 10px;
        border-radius: 10px;
        touch-action: none;       
      }
    </style>
    </head>
    <body>
      <h2>Car control</h2>
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

#endif
