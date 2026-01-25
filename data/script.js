// ===================================
// Auto-refresh dla Dashboard
// ===================================
let refreshInterval;

// Funkcja pobierania statusu z API
async function fetchStatus() {
  try {
    const response = await fetch('/api/status');
    const data = await response.json();
    
    // Aktualizacja wartości
    document.getElementById('tempCO').textContent = data.tempCO.toFixed(1);
    document.getElementById('tempEXT').textContent = data.tempEXT.toFixed(1);
    document.getElementById('tempTarget').textContent = data.tempTarget.toFixed(1);
    document.getElementById('actuatorPos').textContent = data.actuatorPos;
    
    // Status badges
    updateBadge('thermostatStatus', data.thermostatActive, 'Aktywny', 'Nieaktywny');
    updateBadge('modbusStatus', data.modbusConnected, 'Połączony', 'Rozłączony');
    
    // WiFi signal strength
    let wifiQuality = 'Słaby';
    if (data.wifiRSSI > -50) wifiQuality = 'Doskonały';
    else if (data.wifiRSSI > -60) wifiQuality = 'Bardzo dobry';
    else if (data.wifiRSSI > -70) wifiQuality = 'Dobry';
    else if (data.wifiRSSI > -80) wifiQuality = 'Średni';
    
    document.getElementById('wifiStatus').textContent = wifiQuality + ' (' + data.wifiRSSI + ' dBm)';
    document.getElementById('wifiStatus').className = data.wifiRSSI > -70 ? 'badge success' : 'badge warning';
    
    // Uptime
    const hours = Math.floor(data.uptime / 3600);
    const minutes = Math.floor((data.uptime % 3600) / 60);
    document.getElementById('uptime').textContent = hours + 'h ' + minutes + 'm';
    
    // Free heap
    document.getElementById('freeHeap').textContent = (data.freeHeap / 1024).toFixed(1) + ' KB';
    
    // Aktualizacja toggle termostatu
    const toggle = document.getElementById('thermostatToggle');
    if (toggle) {
      toggle.checked = data.thermostatActive;
    }
    
  } catch (error) {
    console.error('Błąd pobierania danych:', error);
  }
}

// Pomocnicza funkcja do aktualizacji badge
function updateBadge(id, condition, textTrue, textFalse) {
  const badge = document.getElementById(id);
  if (badge) {
    badge.textContent = condition ? textTrue : textFalse;
    badge.className = condition ? 'badge success' : 'badge error';
  }
}

// Obsługa przełącznika termostatu
document.addEventListener('DOMContentLoaded', function() {
  const toggle = document.getElementById('thermostatToggle');
  if (toggle) {
    toggle.addEventListener('change', async function() {
      try {
        const response = await fetch('/api/thermostat', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            active: this.checked
          })
        });
        
        if (response.ok) {
          console.log('Termostat zaktualizowany');
        } else {
          console.error('Błąd aktualizacji termostatu');
          this.checked = !this.checked; // Przywróć poprzedni stan
        }
      } catch (error) {
        console.error('Błąd:', error);
        this.checked = !this.checked; // Przywróć poprzedni stan
      }
    });
  }
  
  // Start auto-refresh co 5 sekund
  refreshInterval = setInterval(fetchStatus, 5000);
  fetchStatus(); // Pierwsze wywołanie od razu
});
