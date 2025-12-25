// CONFIG: Replace with your Vercel backend URL
const BACKEND_URL = 'https://your-vercel-app.vercel.app';

let acState = { power: false, temp: 24, hasPowerOn: false, hasPowerOff: false };

async function callBackend(endpoint) {
  try {
    const res = await fetch(`${BACKEND_URL}${endpoint}`);
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    return res;
  } catch (err) {
    alert('⚠️ Dashboard error. Check console.');
    console.error('Backend error:', err);
  }
}

function updateUI() {
  const statusEl = document.getElementById('powerStatus');
  statusEl.textContent = acState.power ? 'ON' : 'OFF';
  statusEl.className = acState.power ? 'on' : 'off';

  document.getElementById('tempValue').textContent = `${acState.temp}°C`;

  // Enable/disable buttons
  const canControl = acState.hasPowerOn && acState.hasPowerOff;
  document.getElementById('powerBtn').disabled = !canControl;
  document.getElementById('powerBtn').textContent = acState.power ? 'TURN OFF' : 'TURN ON';
  document.getElementById('powerBtn').className = `btn ${acState.power ? 'power-off' : 'power'}`;

  document.getElementById('tempUp').disabled = !acState.power || acState.temp >= 30;
  document.getElementById('tempDown').disabled = !acState.power || acState.temp <= 16;
}

async function fetchStatus() {
  try {
    const res = await callBackend('/api/status');
    const data = await res.json();
    acState = data;
    updateUI();
  } catch (e) {
    // Silent retry
  }
}

// Event Listeners
document.getElementById('powerBtn').onclick = () =>
  callBackend(`/api/power?state=${acState.power ? 'off' : 'on'}`).then(fetchStatus);

document.getElementById('tempUp').onclick = () =>
  callBackend(`/api/temp?temp=${acState.temp + 1}`).then(fetchStatus);

document.getElementById('tempDown').onclick = () =>
  callBackend(`/api/temp?temp=${acState.temp - 1}`).then(fetchStatus);

function startLearning(type, label) {
  const msg = document.getElementById('learnMsg');
  msg.textContent = `▶️ Learning ${label}... Press button on your remote!`;
  msg.classList.remove('hidden');

  callBackend(`/api/learn/${type}`)
    .then(() => {
      setTimeout(() => {
        callBackend(`/api/save/${type}`)
          .then(() => {
            msg.textContent = `✅ Saved ${label}!`;
            fetchStatus();
          })
          .catch(() => msg.textContent = `❌ Failed to save ${label}`);
        setTimeout(() => msg.classList.add('hidden'), 3000);
      }, 5000);
    });
}

document.getElementById('learnOn').onclick = () => startLearning('poweron', 'POWER ON');
document.getElementById('learnOff').onclick = () => startLearning('poweroff', 'POWER OFF');

// Initialize
fetchStatus();
setInterval(fetchStatus, 2000);