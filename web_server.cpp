#include "web_server.h"
#include "config.h"
#include "modbus_handler.h"
#include "thermostat.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// ===================================
// Obiekt serwera WWW
// ===================================
AsyncWebServer server(80);

// ===================================
// HTML wbudowany jako PROGMEM
// ===================================

// Dashboard - Główna strona z wyświetlaniem danych w czasie rzeczywistym
// Odświeżanie co 2 sekundy, pokazuje temperatury, status Modbus, siłownik
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OPEC ESP32 - Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Arial, sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: #333;
      padding: 20px;
    }
    .container { max-width: 1200px; margin: 0 auto; }
    .header { 
      text-align: center; 
      color: white; 
      margin-bottom: 30px;
      padding: 20px;
      background: rgba(255,255,255,0.1);
      border-radius: 10px;
    }
    .header h1 { font-size: 2.5em; margin-bottom: 10px; }
    .header p { font-size: 1.1em; opacity: 0.9; }
    
    .card { 
      background: white; 
      border-radius: 15px; 
      padding: 25px; 
      margin-bottom: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
      transition: transform 0.3s;
    }
    .card:hover { transform: translateY(-5px); }
    
    .status-badge {
      display: inline-block;
      padding: 8px 16px;
      border-radius: 20px;
      font-weight: bold;
      font-size: 0.9em;
    }
    .status-connected { background: #10b981; color: white; }
    .status-disconnected { background: #ef4444; color: white; }
    
    .temp-display {
      font-size: 3em;
      font-weight: bold;
      color: #667eea;
      text-align: center;
      margin: 20px 0;
    }
    .temp-label {
      font-size: 1.2em;
      color: #666;
      text-align: center;
      margin-bottom: 10px;
    }
    
    .grid { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); 
      gap: 20px; 
    }
    
    .progress-bar {
      width: 100%;
      height: 30px;
      background: #e5e7eb;
      border-radius: 15px;
      overflow: hidden;
      margin: 10px 0;
    }
    .progress-fill {
      height: 100%;
      background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
      transition: width 0.5s;
      display: flex;
      align-items: center;
      justify-content: center;
      color: white;
      font-weight: bold;
    }
    
    nav {
      background: white;
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 20px;
      text-align: center;
    }
    nav a {
      display: inline-block;
      padding: 10px 20px;
      margin: 5px;
      background: #667eea;
      color: white;
      text-decoration: none;
      border-radius: 8px;
      transition: background 0.3s;
    }
    nav a:hover { background: #764ba2; }
    
    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 12px 0;
      border-bottom: 1px solid #e5e7eb;
    }
    .info-row:last-child { border-bottom: none; }
    .info-label { font-weight: 600; color: #666; }
    .info-value { color: #333; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🏠 OPEC ESP32</h1>
      <p>Sterownik systemu grzewczego</p>
    </div>
    
    <nav>
      <a href="/">Dashboard</a>
      <a href="/settings.html">Ustawienia</a>
      <a href="/curve.html">Krzywa grzewcza</a>
      <a href="/diagnostics.html">Diagnostyka</a>
    </nav>
    
    <div class="card">
      <h2>Status systemu</h2>
      <div style="margin-top: 15px;">
        <span class="status-badge" id="statusBadge">⏳ Łączenie...</span>
      </div>
      <div class="info-row">
        <span class="info-label">Czas pracy:</span>
        <span class="info-value" id="uptime">--</span>
      </div>
    </div>
    
    <div class="grid">
      <div class="card">
        <div class="temp-label">🔥 Temperatura CO</div>
        <div class="temp-display" id="tempCO">--°C</div>
      </div>
      
      <div class="card">
        <div class="temp-label">🌡️ Temperatura zewnętrzna</div>
        <div class="temp-display" id="tempEXT">--°C</div>
      </div>
      
      <div class="card">
        <div class="temp-label">🎯 Temperatura zadana</div>
        <div class="temp-display" id="targetTemp">--°C</div>
      </div>
    </div>
    
    <div class="card">
      <h2>⚙️ Siłownik</h2>
      <div class="progress-bar">
        <div class="progress-fill" id="actuatorBar" style="width: 0%">
          <span id="actuatorText">0%</span>
        </div>
      </div>
    </div>
  </div>
  
  <script>
    function formatUptime(seconds) {
      const h = Math.floor(seconds / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      const s = seconds % 60;
      return h + 'h ' + m + 'm ' + s + 's';
    }
    
    function updateData() {
      fetch('/api/status')
        .then(response => response.json())
        .then(data => {
          // Temperatury
          document.getElementById('tempCO').innerText = data.tempCO.toFixed(1) + '°C';
          document.getElementById('tempEXT').innerText = data.tempEXT.toFixed(1) + '°C';
          document.getElementById('targetTemp').innerText = data.targetTemp.toFixed(1) + '°C';
          
          // Status Modbus
          const badge = document.getElementById('statusBadge');
          if (data.modbusConnected) {
            badge.className = 'status-badge status-connected';
            badge.innerText = '✅ Połączono z PLC';
          } else {
            badge.className = 'status-badge status-disconnected';
            badge.innerText = '❌ Brak połączenia Modbus';
          }
          
          // Siłownik
          document.getElementById('actuatorBar').style.width = data.actuatorPos + '%';
          document.getElementById('actuatorText').innerText = data.actuatorPos + '%';
          
          // Uptime
          document.getElementById('uptime').innerText = formatUptime(data.uptime);
        })
        .catch(err => {
          console.error('Błąd odczytu danych:', err);
        });
    }
    
    // Odświeżanie co 2 sekundy
    updateData();
    setInterval(updateData, 2000);
  </script>
</body>
</html>
)rawliteral";

// Ustawienia - Strona konfiguracji WiFi, Modbus i termostatu
// Formularz z zapisem do Preferences i opcjonalnym restartem
const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OPEC ESP32 - Ustawienia</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Arial, sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      padding: 20px;
    }
    .container { max-width: 800px; margin: 0 auto; }
    .header { 
      text-align: center; 
      color: white; 
      margin-bottom: 30px;
      padding: 20px;
    }
    .card { 
      background: white; 
      border-radius: 15px; 
      padding: 30px; 
      margin-bottom: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    }
    h2 { color: #667eea; margin-bottom: 20px; }
    
    .form-group { margin-bottom: 20px; }
    label { 
      display: block; 
      font-weight: 600; 
      margin-bottom: 8px;
      color: #333;
    }
    input, select {
      width: 100%;
      padding: 12px;
      border: 2px solid #e5e7eb;
      border-radius: 8px;
      font-size: 1em;
      transition: border 0.3s;
    }
    input:focus, select:focus {
      outline: none;
      border-color: #667eea;
    }
    
    button {
      padding: 12px 30px;
      background: #667eea;
      color: white;
      border: none;
      border-radius: 8px;
      font-size: 1em;
      font-weight: 600;
      cursor: pointer;
      transition: background 0.3s;
      margin-right: 10px;
    }
    button:hover { background: #764ba2; }
    button.danger { background: #ef4444; }
    button.danger:hover { background: #dc2626; }
    
    nav {
      background: white;
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 20px;
      text-align: center;
    }
    nav a {
      display: inline-block;
      padding: 10px 20px;
      margin: 5px;
      background: #667eea;
      color: white;
      text-decoration: none;
      border-radius: 8px;
    }
    nav a:hover { background: #764ba2; }
    
    .checkbox-group {
      display: flex;
      align-items: center;
      gap: 10px;
    }
    input[type="checkbox"] {
      width: auto;
      transform: scale(1.5);
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>⚙️ Ustawienia</h1>
    </div>
    
    <nav>
      <a href="/">Dashboard</a>
      <a href="/settings.html">Ustawienia</a>
      <a href="/curve.html">Krzywa grzewcza</a>
      <a href="/diagnostics.html">Diagnostyka</a>
    </nav>
    
    <div class="card">
      <h2>📡 Konfiguracja WiFi</h2>
      <div class="form-group">
        <label>Nazwa sieci (SSID):</label>
        <input type="text" id="wifiSSID" placeholder="Wprowadź nazwę sieci WiFi">
      </div>
      <div class="form-group">
        <label>Hasło WiFi:</label>
        <input type="password" id="wifiPass" placeholder="Wprowadź hasło WiFi">
      </div>
      <p style="color: #666; font-size: 0.9em;">
        ℹ️ Po zapisaniu ESP32 zrestartuje się i połączy z podaną siecią.
      </p>
    </div>
    
    <div class="card">
      <h2>🔌 Konfiguracja Modbus</h2>
      <div class="form-group">
        <label>Baudrate:</label>
        <select id="modbusBaud">
          <option value="9600">9600</option>
          <option value="19200">19200</option>
          <option value="38400">38400</option>
          <option value="57600">57600</option>
          <option value="115200" selected>115200</option>
        </select>
      </div>
      <div class="form-group">
        <label>Unit ID (Slave ID):</label>
        <input type="number" id="modbusID" min="1" max="247" value="5">
      </div>
    </div>
    
    <div class="card">
      <h2>🌡️ Ustawienia termostatu</h2>
      <div class="form-group">
        <label>Histereza (°C):</label>
        <input type="number" id="hysteresis" step="0.1" min="0.5" max="5.0" value="2.0">
      </div>
      <div class="form-group checkbox-group">
        <input type="checkbox" id="thermoActive" checked>
        <label for="thermoActive">Termostat aktywny</label>
      </div>
    </div>
    
    <div class="card">
      <button onclick="saveSettings()">💾 Zapisz ustawienia</button>
      <button class="danger" onclick="resetSettings()">🔄 Reset fabryczny</button>
    </div>
  </div>
  
  <script>
    // Wczytaj obecne ustawienia
    function loadSettings() {
      fetch('/api/settings')
        .then(r => r.json())
        .then(data => {
          document.getElementById('wifiSSID').value = data.wifiSSID || '';
          document.getElementById('modbusBaud').value = data.modbusBaudrate || 115200;
          document.getElementById('modbusID').value = data.modbusUnitID || 5;
          document.getElementById('hysteresis').value = data.hysteresis || 2.0;
          document.getElementById('thermoActive').checked = data.thermostatActive !== false;
        })
        .catch(err => console.error('Błąd wczytywania ustawień:', err));
    }
    
    function saveSettings() {
      const data = {
        wifiSSID: document.getElementById('wifiSSID').value,
        wifiPass: document.getElementById('wifiPass').value,
        modbusBaud: parseInt(document.getElementById('modbusBaud').value),
        modbusID: parseInt(document.getElementById('modbusID').value),
        hysteresis: parseFloat(document.getElementById('hysteresis').value),
        thermoActive: document.getElementById('thermoActive').checked
      };
      
      fetch('/api/settings', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      })
      .then(response => response.json())
      .then(result => {
        alert('✅ Ustawienia zapisane! ESP32 zrestartuje się za 3 sekundy.');
      })
      .catch(err => {
        alert('❌ Błąd zapisu: ' + err);
      });
    }
    
    function resetSettings() {
      if (confirm('Czy na pewno chcesz przywrócić ustawienia fabryczne?')) {
        fetch('/api/reset', { method: 'POST' })
        .then(() => {
          alert('✅ Ustawienia zresetowane! ESP32 zrestartuje się.');
        });
      }
    }
    
    loadSettings();
  </script>
</body>
</html>
)rawliteral";

// Krzywa grzewcza - Edycja 9 punktów krzywej temperaturowej
// Tabela z polami input, zapisuje do Preferences
const char curve_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OPEC ESP32 - Krzywa grzewcza</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Arial, sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      padding: 20px;
    }
    .container { max-width: 900px; margin: 0 auto; }
    .header { text-align: center; color: white; margin-bottom: 30px; padding: 20px; }
    .card { 
      background: white; 
      border-radius: 15px; 
      padding: 30px; 
      margin-bottom: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    }
    h2 { color: #667eea; margin-bottom: 20px; }
    
    table { width: 100%; border-collapse: collapse; margin: 20px 0; }
    th, td { padding: 12px; text-align: center; border-bottom: 1px solid #e5e7eb; }
    th { background: #f3f4f6; font-weight: 600; }
    input[type="number"] {
      width: 100px;
      padding: 8px;
      border: 2px solid #e5e7eb;
      border-radius: 6px;
      text-align: center;
    }
    
    button {
      padding: 12px 30px;
      background: #667eea;
      color: white;
      border: none;
      border-radius: 8px;
      font-size: 1em;
      font-weight: 600;
      cursor: pointer;
      margin-right: 10px;
    }
    button:hover { background: #764ba2; }
    
    nav {
      background: white;
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 20px;
      text-align: center;
    }
    nav a {
      display: inline-block;
      padding: 10px 20px;
      margin: 5px;
      background: #667eea;
      color: white;
      text-decoration: none;
      border-radius: 8px;
    }
    nav a:hover { background: #764ba2; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>📈 Krzywa grzewcza</h1>
    </div>
    
    <nav>
      <a href="/">Dashboard</a>
      <a href="/settings.html">Ustawienia</a>
      <a href="/curve.html">Krzywa grzewcza</a>
      <a href="/diagnostics.html">Diagnostyka</a>
    </nav>
    
    <div class="card">
      <h2>Edycja krzywej (9 punktów)</h2>
      <table>
        <thead>
          <tr>
            <th>Punkt</th>
            <th>Temp. zewnętrzna (°C)</th>
            <th>Temp. CO (°C)</th>
          </tr>
        </thead>
        <tbody id="curveTable">
          <!-- Wypełnione przez JavaScript -->
        </tbody>
      </table>
      
      <button onclick="saveCurve()">💾 Zapisz krzywą</button>
      <button onclick="loadDefaultCurve()">🔄 Przywróć domyślną</button>
    </div>
  </div>
  
  <script>
    const defaultCurve = [
      [-20, 65], [-15, 60], [-10, 55], [-5, 50], [0, 45],
      [5, 40], [10, 35], [15, 30], [20, 25]
    ];
    
    function loadCurve() {
      fetch('/api/curve')
        .then(r => r.json())
        .then(data => {
          renderCurve(data.curve || defaultCurve);
        })
        .catch(() => {
          renderCurve(defaultCurve);
        });
    }
    
    function renderCurve(curve) {
      const tbody = document.getElementById('curveTable');
      tbody.innerHTML = '';
      curve.forEach((point, i) => {
        tbody.innerHTML += `
          <tr>
            <td>${i + 1}</td>
            <td><input type="number" id="ext_${i}" value="${point[0]}" step="1"></td>
            <td><input type="number" id="co_${i}" value="${point[1]}" step="0.5"></td>
          </tr>
        `;
      });
    }
    
    function saveCurve() {
      const curve = [];
      for (let i = 0; i < 9; i++) {
        curve.push([
          parseFloat(document.getElementById('ext_' + i).value),
          parseFloat(document.getElementById('co_' + i).value)
        ]);
      }
      
      fetch('/api/curve', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ curve })
      })
      .then(() => alert('✅ Krzywa zapisana!'))
      .catch(err => alert('❌ Błąd: ' + err));
    }
    
    function loadDefaultCurve() {
      if (confirm('Przywrócić domyślną krzywą grzewczą?')) {
        renderCurve(defaultCurve);
      }
    }
    
    loadCurve();
  </script>
</body>
</html>
)rawliteral";

// Diagnostyka - Strona diagnostyki Modbus z surowymi danymi z rejestrów
// Testowanie komunikacji z czujnikiem HT73/SHT35 (Slave ID: 5)
const char diagnostics_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OPEC ESP32 - Diagnostyka Modbus</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', Arial, sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      padding: 20px;
    }
    .container { max-width: 1400px; margin: 0 auto; }
    .header { 
      text-align: center; 
      color: white; 
      margin-bottom: 30px;
      padding: 20px;
      background: rgba(255,255,255,0.1);
      border-radius: 10px;
    }
    .header h1 { font-size: 2.5em; margin-bottom: 10px; }
    
    nav {
      background: white;
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 20px;
      text-align: center;
    }
    nav a {
      display: inline-block;
      padding: 10px 20px;
      margin: 5px;
      background: #667eea;
      color: white;
      text-decoration: none;
      border-radius: 8px;
      transition: background 0.3s;
    }
    nav a:hover, nav a.active { background: #764ba2; }
    
    .card { 
      background: white; 
      border-radius: 15px; 
      padding: 25px; 
      margin-bottom: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    }
    .card h2 { margin-bottom: 15px; color: #333; }
    
    .grid { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); 
      gap: 20px; 
    }
    .full-width { grid-column: 1 / -1; }
    
    .form-group {
      margin-bottom: 15px;
    }
    .form-group label {
      display: block;
      margin-bottom: 5px;
      font-weight: 600;
      color: #666;
    }
    .form-group input {
      width: 100%;
      padding: 10px;
      border: 2px solid #e5e7eb;
      border-radius: 8px;
      font-size: 1em;
    }
    
    button {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      padding: 12px 25px;
      border-radius: 8px;
      font-size: 1em;
      font-weight: 600;
      cursor: pointer;
      transition: transform 0.2s;
    }
    button:hover { transform: translateY(-2px); }
    
    .status-row {
      display: flex;
      justify-content: space-between;
      padding: 12px 0;
      border-bottom: 1px solid #e5e7eb;
    }
    .status-row:last-child { border-bottom: none; }
    
    .badge {
      padding: 5px 12px;
      border-radius: 15px;
      font-weight: bold;
      font-size: 0.9em;
    }
    .status-connected { background: #10b981; color: white; }
    .status-disconnected { background: #ef4444; color: white; }
    
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 15px;
    }
    table th, table td {
      padding: 12px;
      text-align: left;
      border-bottom: 1px solid #e5e7eb;
    }
    table th {
      background: #f3f4f6;
      font-weight: 600;
      color: #666;
    }
    
    .temp-display {
      font-size: 3em;
      font-weight: bold;
      color: #667eea;
      text-align: center;
      margin: 20px 0;
    }
    
    #modbusLogs {
      background: #f5f5f5;
      padding: 15px;
      border-radius: 8px;
      font-family: monospace;
      font-size: 0.9em;
      max-height: 300px;
      overflow-y: auto;
      line-height: 1.6;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🔍 Diagnostyka Modbus</h1>
    </div>
    
    <nav>
      <a href="/">Dashboard</a>
      <a href="/settings.html">Ustawienia</a>
      <a href="/curve.html">Krzywa grzewcza</a>
      <a href="/diagnostics.html" class="active">Diagnostyka</a>
    </nav>
    
    <main>
      <div class="grid">
        <!-- Panel sterowania testem -->
        <div class="card">
          <h2>🔧 Test komunikacji Modbus</h2>
          <div class="form-group">
            <label>Slave ID:</label>
            <input type="number" id="testSlaveID" min="1" max="247" value="5">
          </div>
          <div class="form-group">
            <label>Rejestr startowy:</label>
            <input type="number" id="testStartReg" min="0" max="65535" value="0">
          </div>
          <div class="form-group">
            <label>Liczba rejestrów:</label>
            <input type="number" id="testCount" min="1" max="10" value="5">
          </div>
          <button onclick="testModbus()">🚀 Wykonaj test</button>
        </div>

        <!-- Status połączenia -->
        <div class="card">
          <h2>📊 Status połączenia</h2>
          <div class="status-row">
            <span>Slave ID:</span>
            <span id="currentSlaveID">--</span>
          </div>
          <div class="status-row">
            <span>Status:</span>
            <span id="modbusStatus" class="badge">--</span>
          </div>
          <div class="status-row">
            <span>Ostatni odczyt:</span>
            <span id="lastRead">--</span>
          </div>
        </div>

        <!-- Interpretacja dla czujnika HT73/SHT35 -->
        <div class="card">
          <h2>🌡️ Odczyt temperatury (HT73/SHT35)</h2>
          <div class="temp-display" id="sensorTemp">--</div>
          <p style="text-align: center; color: #666;">
            Temperatura z rejestru 2 (wartość/10)
          </p>
        </div>
      </div>

      <!-- Tabela surowych rejestrów -->
      <div class="card full-width">
        <h2>📋 Surowe wartości rejestrów</h2>
        <table>
          <thead>
            <tr>
              <th>Rejestr</th>
              <th>HEX</th>
              <th>Decimal</th>
              <th>Binary</th>
              <th>Int16</th>
              <th>Float (/10)</th>
              <th>Opis</th>
            </tr>
          </thead>
          <tbody id="registersTable">
            <tr><td colspan="7" style="text-align: center;">Brak danych...</td></tr>
          </tbody>
        </table>
      </div>

      <!-- Logi komunikacji -->
      <div class="card full-width">
        <h2>📝 Logi komunikacji</h2>
        <div id="modbusLogs">Brak logów...</div>
        <button onclick="clearLogs()" style="margin-top: 10px;">🗑️ Wyczyść logi</button>
      </div>
    </main>
  </div>

  <script>
    let logs = [];
    
    function addLog(message) {
      const timestamp = new Date().toLocaleTimeString();
      logs.unshift(`[${timestamp}] ${message}`);
      if (logs.length > 50) logs.pop();
      document.getElementById('modbusLogs').innerHTML = logs.join('<br>');
    }
    
    function clearLogs() {
      logs = [];
      document.getElementById('modbusLogs').innerHTML = 'Brak logów...';
    }
    
    async function testModbus() {
      const slaveID = document.getElementById('testSlaveID').value;
      const startReg = document.getElementById('testStartReg').value;
      const count = document.getElementById('testCount').value;
      
      addLog(`Wysyłanie testu Modbus: Slave=${slaveID}, Start=${startReg}, Count=${count}`);
      
      try {
        const response = await fetch('/api/modbus-test', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({
            slaveID: parseInt(slaveID),
            startRegister: parseInt(startReg),
            count: parseInt(count)
          })
        });
        
        const data = await response.json();
        addLog(`Odczyt ${data.connected ? 'SUKCES' : 'BŁĄD'}`);
        updateDisplay(data);
      } catch (error) {
        addLog(`Błąd: ${error.message}`);
      }
    }
    
    function updateDisplay(data) {
      document.getElementById('currentSlaveID').textContent = data.slaveID || '--';
      document.getElementById('modbusStatus').textContent = data.connected ? 'Połączony' : 'Błąd';
      document.getElementById('modbusStatus').className = data.connected ? 'badge status-connected' : 'badge status-disconnected';
      document.getElementById('lastRead').textContent = new Date().toLocaleTimeString();
      
      // Wypełnij tabelę rejestrów
      const tbody = document.getElementById('registersTable');
      tbody.innerHTML = '';
      
      if (data.registers && data.registers.length > 0) {
        data.registers.forEach(reg => {
          const row = tbody.insertRow();
          const binary = reg.decimal.toString(2).padStart(16, '0');
          const int16 = reg.decimal > 32767 ? reg.decimal - 65536 : reg.decimal;
          const float = (int16 / 10.0).toFixed(1);
          
          row.innerHTML = `
            <td><strong>${reg.index}</strong></td>
            <td style="font-family: monospace;">${reg.hex}</td>
            <td>${reg.decimal}</td>
            <td style="font-family: monospace; font-size: 0.8em;">${binary}</td>
            <td>${int16}</td>
            <td>${float}</td>
            <td>${reg.description || ''}</td>
          `;
        });
      } else {
        tbody.innerHTML = '<tr><td colspan="7" style="text-align: center;">Brak danych</td></tr>';
      }
      
      // Temperatura
      if (data.interpretedTemp !== undefined && data.interpretedTemp !== null) {
        document.getElementById('sensorTemp').textContent = data.interpretedTemp.toFixed(1) + '°C';
        addLog(`Temperatura odczytana: ${data.interpretedTemp.toFixed(1)}°C`);
      } else {
        document.getElementById('sensorTemp').textContent = '--';
      }
    }
    
    // Auto-refresh co 5 sekund
    let autoRefreshEnabled = true;
    setInterval(() => {
      if (autoRefreshEnabled) {
        testModbus();
      }
    }, 5000);
    
    // Pierwszy odczyt po załadowaniu
    window.onload = () => {
      addLog('Strona diagnostyki załadowana');
      setTimeout(testModbus, 500);
    };
  </script>
</body>
</html>
)rawliteral";

// ===================================
// Pomocnicza funkcja - generowanie unikalnego SSID
// ===================================
String getAPSSID() {
  uint64_t chipid = ESP.getEfuseMac();
  char ssid[32];
  snprintf(ssid, sizeof(ssid), "%s%04X", AP_SSID_PREFIX, (uint16_t)(chipid & 0xFFFF));
  return String(ssid);
}

// ===================================
// Konfiguracja WiFi (AP lub STA mode)
// ===================================
void setupWiFi() {
  // Sprawdź czy jest zapisana konfiguracja WiFi
  if (config.configInitialized && strlen(config.wifiSSID) > 0) {
    // Tryb STA - połącz z zapisaną siecią
    Serial.printf("[WiFi] Łączenie z siecią: %s\n", config.wifiSSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifiSSID, config.wifiPassword);
    
    // Czekaj max 20 sekund na połączenie
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.printf("[WiFi] Połączono! IP: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
      return;
    } else {
      Serial.println();
      Serial.println("[WiFi] Nie udało się połączyć, uruchamiam tryb AP");
    }
  }
  
  // Tryb AP - utwórz własny punkt dostępu
  String apSSID = getAPSSID();
  Serial.printf("[WiFi] Uruchamiam Access Point: %s\n", apSSID.c_str());
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(apSSID.c_str(), AP_PASSWORD);
  
  Serial.printf("[WiFi] AP uruchomiony! IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("[WiFi] Hasło: %s\n", AP_PASSWORD);
}

// ===================================
// Handler dla stron HTML
// ===================================
void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", index_html);
}

void handleSettings(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", settings_html);
}

void handleCurve(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", curve_html);
}

void handleDiagnostics(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", diagnostics_html);
}

// ===================================
// Konfiguracja serwera WWW i endpointów API
// ===================================
void setupWebServer() {
  // Routing HTML stron (wbudowanych w kod)
  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);
  server.on("/settings.html", HTTP_GET, handleSettings);
  server.on("/curve.html", HTTP_GET, handleCurve);
  server.on("/diagnostics.html", HTTP_GET, handleDiagnostics);
  
  Serial.println("[WebServer] Strony HTML wbudowane w kod (bez SPIFFS)");
  
  // ===================================
  // API Endpoint: GET /api/status
  // Zwraca aktualny stan systemu w JSON
  // ===================================
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    
    doc["tempCO"] = tempCO;
    doc["tempEXT"] = tempEXT;
    doc["targetTemp"] = targetTemp;  // Poprawiona nazwa z tempTarget -> targetTemp
    doc["actuatorPos"] = actuatorPos;
    doc["thermostatActive"] = config.thermostatActive;
    doc["modbusConnected"] = modbusConnected;
    doc["wifiRSSI"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["chipModel"] = ESP.getChipModel();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: POST /api/curve
  // Aktualizacja krzywej grzewczej
  // ===================================
  server.on("/api/curve", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        request->send(400, "text/plain", "Błąd parsowania JSON");
        return;
      }
      
      // Aktualizacja krzywej
      JsonArray curve = doc["curve"];
      if (curve.size() == HEATING_CURVE_POINTS) {
        for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
          config.heatingCurve[i][0] = curve[i][0];
          config.heatingCurve[i][1] = curve[i][1];
        }
        saveConfig();
        Serial.println("[WebServer] Krzywa grzewcza zaktualizowana");
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Nieprawidłowa liczba punktów krzywej");
      }
    }
  );
  
  // ===================================
  // API Endpoint: GET /api/curve
  // Pobieranie krzywej grzewczej
  // ===================================
  server.on("/api/curve", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<1024> doc;
    JsonArray curve = doc.createNestedArray("curve");
    
    for (int i = 0; i < HEATING_CURVE_POINTS; i++) {
      JsonArray point = curve.createNestedArray();
      point.add(config.heatingCurve[i][0]);
      point.add(config.heatingCurve[i][1]);
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: POST /api/thermostat
  // Włączenie/wyłączenie termostatu
  // ===================================
  server.on("/api/thermostat", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        request->send(400, "text/plain", "Błąd parsowania JSON");
        return;
      }
      
      if (doc.containsKey("active")) {
        config.thermostatActive = doc["active"];
        saveConfig();
        Serial.printf("[WebServer] Termostat %s\n", 
                      config.thermostatActive ? "WŁĄCZONY" : "WYŁĄCZONY");
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Brak parametru 'active'");
      }
    }
  );
  
  // ===================================
  // API Endpoint: POST /api/settings
  // Aktualizacja ustawień
  // ===================================
  server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        StaticJsonDocument<128> resp;
        resp["status"] = "error";
        resp["message"] = "Błąd parsowania JSON";
        String response;
        serializeJson(resp, response);
        request->send(400, "application/json", response);
        return;
      }
      
      bool wifiChanged = false;
      
      // Aktualizacja WiFi (obsługa zarówno wifiSSID jak i wifiPass)
      if (doc.containsKey("wifiSSID")) {
        strlcpy(config.wifiSSID, doc["wifiSSID"], sizeof(config.wifiSSID));
        wifiChanged = true;
      }
      if (doc.containsKey("wifiPassword")) {
        strlcpy(config.wifiPassword, doc["wifiPassword"], sizeof(config.wifiPassword));
        wifiChanged = true;
      }
      if (doc.containsKey("wifiPass")) {
        strlcpy(config.wifiPassword, doc["wifiPass"], sizeof(config.wifiPassword));
        wifiChanged = true;
      }
      
      // Aktualizacja Modbus (obsługa zarówno modbusUnitID jak i modbusID)
      if (doc.containsKey("modbusUnitID")) {
        config.modbusUnitID = doc["modbusUnitID"];
      }
      if (doc.containsKey("modbusID")) {
        config.modbusUnitID = doc["modbusID"];
      }
      if (doc.containsKey("modbusBaudrate")) {
        config.modbusBaudrate = doc["modbusBaudrate"];
      }
      if (doc.containsKey("modbusBaud")) {
        config.modbusBaudrate = doc["modbusBaud"];
      }
      
      // Aktualizacja histerezy
      if (doc.containsKey("hysteresis")) {
        config.hysteresis = doc["hysteresis"];
      }
      
      // Aktualizacja statusu termostatu (obsługa thermoActive)
      if (doc.containsKey("thermostatActive")) {
        config.thermostatActive = doc["thermostatActive"];
      }
      if (doc.containsKey("thermoActive")) {
        config.thermostatActive = doc["thermoActive"];
      }
      
      saveConfig();
      Serial.println("[WebServer] Ustawienia zaktualizowane");
      
      StaticJsonDocument<128> resp;
      resp["status"] = "ok";
      resp["message"] = wifiChanged ? "Zapisano - restart za 3s" : "Zapisano";
      String response;
      serializeJson(resp, response);
      request->send(200, "application/json", response);
      
      // Restart po 3 sekundach (jeśli zmieniono WiFi) - używamy Task zamiast delay
      if (wifiChanged) {
        static bool restartScheduled = false;
        if (!restartScheduled) {
          restartScheduled = true;
          // Zaplanuj restart w osobnym zadaniu
          xTaskCreate([](void* param) {
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            ESP.restart();
          }, "restart_task", 2048, NULL, 1, NULL);
        }
      }
    }
  );
  
  // ===================================
  // API Endpoint: GET /api/settings
  // Pobieranie ustawień
  // ===================================
  server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    
    doc["wifiSSID"] = config.wifiSSID;
    // Nie wysyłamy hasła WiFi
    doc["modbusUnitID"] = config.modbusUnitID;
    doc["modbusBaudrate"] = config.modbusBaudrate;
    doc["hysteresis"] = config.hysteresis;
    doc["thermostatActive"] = config.thermostatActive;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: GET /api/modbus-raw
  // Zwraca surowe dane z rejestrów Modbus
  // ===================================
  server.on("/api/modbus-raw", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<1024> doc;
    
    doc["slaveID"] = config.modbusUnitID;
    doc["connected"] = modbusConnected;
    
    JsonArray registers = doc.createNestedArray("registers");
    for (int i = 0; i < 6; i++) {
      JsonObject reg = registers.createNestedObject();
      reg["index"] = i;
      
      char hexStr[8];
      snprintf(hexStr, sizeof(hexStr), "0x%04X", modbusRawRegisters[i]);
      reg["hex"] = hexStr;
      
      reg["decimal"] = modbusRawRegisters[i];
      
      // Dodaj opis dla konkretnych rejestrów
      if (i == 0) {
        reg["description"] = "Rejestr 0";
      } else if (i == 1) {
        reg["description"] = "Rejestr 1";
      } else if (i == 2) {
        reg["description"] = "Rejestr 2 (Temp*10)";
      } else if (i == 3) {
        reg["description"] = "Rejestr 3";
      } else if (i == 4) {
        reg["description"] = "Rejestr 4 (TempEXT*10)";
      } else if (i == 5) {
        reg["description"] = "Rejestr 5 (TempCO*10)";
      }
    }
    
    // Interpretacja temperatury z rejestru 2 (dla czujnika HT73/SHT35)
    int16_t tempRaw = (int16_t)modbusRawRegisters[2];
    doc["interpretedTemp"] = tempRaw / 10.0;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // ===================================
  // API Endpoint: POST /api/modbus-test
  // Testowy odczyt z niestandardowym Slave ID
  // ===================================
  server.on("/api/modbus-test", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, data, len);
      
      if (error) {
        request->send(400, "text/plain", "Błąd parsowania JSON");
        return;
      }
      
      // Odczyt parametrów
      uint8_t slaveID = doc["slaveID"] | 5;
      uint16_t startRegister = doc["startRegister"] | 0;
      uint16_t count = doc["count"] | 5;
      
      // Walidacja
      if (slaveID < 1 || slaveID > 247 || count < 1 || count > 125) {
        request->send(400, "text/plain", "Nieprawidłowe parametry");
        return;
      }
      
      // Wykonaj odczyt
      bool success = readModbusRawData(slaveID, startRegister, count);
      
      // Przygotuj odpowiedź
      StaticJsonDocument<1024> response;
      response["slaveID"] = slaveID;
      response["connected"] = success;
      
      JsonArray registers = response.createNestedArray("registers");
      for (int i = 0; i < count && i < 6; i++) {
        JsonObject reg = registers.createNestedObject();
        reg["index"] = startRegister + i;
        
        char hexStr[8];
        snprintf(hexStr, sizeof(hexStr), "0x%04X", modbusRawRegisters[i]);
        reg["hex"] = hexStr;
        
        reg["decimal"] = modbusRawRegisters[i];
        
        // Dodaj opis
        if (startRegister + i == 2) {
          reg["description"] = "Rejestr 2 (Temp*10)";
        } else {
          char desc[32];
          snprintf(desc, sizeof(desc), "Rejestr %d", startRegister + i);
          reg["description"] = desc;
        }
      }
      
      // Interpretacja temperatury z rejestru 2
      if (startRegister <= 2 && (startRegister + count) > 2) {
        int regIndex = 2 - startRegister;
        int16_t tempRaw = (int16_t)modbusRawRegisters[regIndex];
        response["interpretedTemp"] = tempRaw / 10.0;
      } else {
        response["interpretedTemp"] = nullptr;
      }
      
      String responseStr;
      serializeJson(response, responseStr);
      request->send(200, "application/json", responseStr);
    }
  );
  
  // ===================================
  // API Endpoint: POST /api/reset
  // Reset ustawień do domyślnych
  // ===================================
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("[WebServer] Reset konfiguracji do domyślnych");
    resetConfig();
    request->send(200, "text/plain", "Resetowanie... Urządzenie zostanie zrestartowane");
    delay(1000);
    ESP.restart();
  });
  
  // Uruchomienie serwera
  server.begin();
  Serial.println("[WebServer] Serwer HTTP uruchomiony na porcie 80");
  
  // Konfiguracja mDNS
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] Dostępny pod: http://%s.local\n", MDNS_HOSTNAME);
  } else {
    Serial.println("[mDNS] Błąd inicjalizacji!");
  }
}
