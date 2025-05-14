#ifndef WEBPAGE_H
#define WEBPAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>TOQUEBOX Setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f0f2f5;
    }
    .container {
      max-width: 400px;
      margin: 0 auto;
      background: white;
      padding: 30px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    }
    h2 {
      color: #1a73e8;
      text-align: center;
      margin-bottom: 30px;
      font-weight: 500;
    }
    .form-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #5f6368;
      font-size: 14px;
    }
    input[type=text] {
      width: 100%;
      padding: 12px;
      border: 1px solid #dadce0;
      border-radius: 4px;
      box-sizing: border-box;
      font-size: 16px;
      transition: border-color 0.3s;
    }
    input[type=text]:focus {
      border-color: #1a73e8;
      outline: none;
    }
    button {
      background-color: #1a73e8;
      color: white;
      padding: 14px 20px;
      margin: 8px 0;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      width: 100%;
      font-size: 16px;
      transition: background-color 0.3s;
    }
    button:hover {
      background-color: #1557b0;
    }
    .status {
      text-align: center;
      margin-top: 20px;
      color: #5f6368;
      font-size: 14px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>TOQUEBOX Setup</h2>
    <form action="/save" method="POST">
      <div class="form-group">
        <label for="ssid">WiFi Network Name:</label>
        <input type="text" id="ssid" name="ssid" placeholder="Enter your WiFi SSID">
      </div>
      <div class="form-group">
        <label for="password">WiFi Password:</label>
        <input type="text" id="password" name="password" placeholder="Enter your WiFi password">
      </div>
      <button type="submit">Connect</button>
    </form>
    <div class="status">
      Connect your TOQUEBOX to your WiFi network
    </div>
  </div>
</body>
</html>
)rawliteral";

#endif 
