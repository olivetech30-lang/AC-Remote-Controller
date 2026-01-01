// Configuration
let ESP32_IP = localStorage.getItem('esp32IP') || '192.168.18.230';
let currentTemp = 24;
let isLearningMode = false;

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    loadSettings();
    checkStatus();
    loadLearnedCommands();
    setInterval(checkStatus, 10000); // Check status every 10 seconds
});

// Load settings from localStorage
function loadSettings() {
    const savedIP = localStorage.getItem('esp32IP');
    if (savedIP) {
        ESP32_IP = savedIP;
        document.getElementById('esp32IP').value = savedIP;
    }

    const savedTemp = localStorage.getItem('currentTemp');
    if (savedTemp) {
        currentTemp = parseInt(savedTemp);
        updateTempDisplay(currentTemp);
    }
}

// Save settings to localStorage
function saveSettings() {
    const ipInput = document.getElementById('esp32IP');
    ESP32_IP = ipInput.value.trim();
    localStorage.setItem('esp32IP', ESP32_IP);
    showToast('Settings saved successfully!', 'success');
    checkStatus();
    loadLearnedCommands();
}

// Toggle learning mode
function toggleLearning() {
    isLearningMode = !isLearningMode;
    const learningSection = document.getElementById('learningSection');
    learningSection.style.display = isLearningMode ? 'block' : 'none';
    showToast(isLearningMode ? 'Learning mode enabled' : 'Learning mode disabled', 'warning');
}

// Check ESP32 status
async function checkStatus() {
    try {
        const response = await fetch(`http://${ESP32_IP}/status`);
        const data = await response.json();
        
        document.getElementById('statusText').textContent = `Online (${data.ip})`;
        document.getElementById('status').classList.add('online');
        
        if (data.learning) {
            document.getElementById('learningStatus').textContent = '📡 Waiting for IR signal...';
        }
    } catch (error) {
        document.getElementById('statusText').textContent = 'Offline';
        document.getElementById('status').classList.remove('online');
        console.error('Status check failed:', error);
    }
}

// Load learned commands
async function loadLearnedCommands() {
    try {
        const response = await fetch(`http://${ESP32_IP}/list`);
        const data = await response.json();
        
        const commandsList = document.getElementById('commandsList');
        if (data.commands && data.commands.length > 0) {
            commandsList.innerHTML = data.commands
                .map(cmd => `<span>${cmd}</span>`)
                .join('');
        } else {
            commandsList.innerHTML = '<span style="color: var(--text-secondary);">No commands learned yet</span>';
        }
    } catch (error) {
        document.getElementById('commandsList').innerHTML = '<span style="color: var(--danger-color);">Failed to load commands</span>';
        console.error('Failed to load commands:', error);
    }
}

// Start learning a command
async function startLearning() {
    const select = document.getElementById('learnCommandSelect');
    const command = select.value;
    
    if (!command) {
        showToast('Please select a command to learn', 'warning');
        return;
    }

    try {
        document.getElementById('learningStatus').textContent = `📡 Learning ${command}... Point your AC remote at ESP32 and press the button.`;
        
        const response = await fetch(`http://${ESP32_IP}/learn?cmd=${command}`);
        const data = await response.json();
        
        if (data.status === 'learning') {
            showToast(`Learning ${command}. Press the button on your AC remote now!`, 'warning');
            
            // Wait a bit then check if learned
            setTimeout(() => {
                loadLearnedCommands();
                document.getElementById('learningStatus').textContent = `✅ ${command} learned successfully!`;
                setTimeout(() => {
                    document.getElementById('learningStatus').textContent = '';
                }, 3000);
            }, 3000);
        }
    } catch (error) {
        showToast('Failed to start learning. Check ESP32 connection.', 'error');
        document.getElementById('learningStatus').textContent = '';
        console.error('Learning failed:', error);
    }
}

// Send command to ESP32
async function sendCommand(endpoint, value) {
    try {
        let url;
        if (endpoint === 'power') {
            url = `http://${ESP32_IP}/power?state=${value}`;
        } else if (endpoint === 'temp') {
            url = `http://${ESP32_IP}/temp?temp=${value}`;
        } else if (endpoint === 'mode') {
            url = `http://${ESP32_IP}/mode?mode=${value}`;
        } else if (endpoint === 'fan') {
            url = `http://${ESP32_IP}/fan?speed=${value}`;
        }

        const response = await fetch(url);
        const data = await response.json();

        if (data.status === 'success') {
            showToast(`✅ Command sent: ${endpoint} ${value}`, 'success');
            highlightButton(endpoint, value);
        } else if (data.error) {
            showToast(`❌ ${data.error}`, 'error');
        }
    } catch (error) {
        showToast('Failed to send command. Check ESP32 connection.', 'error');
        console.error('Command failed:', error);
    }
}

// Highlight active button
function highlightButton(type, value) {
    // Remove previous active states
    document.querySelectorAll(`.btn-${type}`).forEach(btn => {
        btn.classList.remove('active');
    });

    // Add active state to clicked button
    event.target.closest('button').classList.add('active');
}

// Temperature controls
function increaseTemp() {
    if (currentTemp < 30) {
        currentTemp++;
        updateTempDisplay(currentTemp);
        sendCommand('temp', currentTemp);
    }
}

function decreaseTemp() {
    if (currentTemp > 16) {
        currentTemp--;
        updateTempDisplay(currentTemp);
        sendCommand('temp', currentTemp);
    }
}

function updateTemp(value) {
    currentTemp = parseInt(value);
    updateTempDisplay(currentTemp);
}

function updateTempDisplay(temp) {
    document.getElementById('tempValue').textContent = temp;
    document.getElementById('tempSlider').value = temp;
    localStorage.setItem('currentTemp', temp);
}

// Apply temperature when slider is released
document.getElementById('tempSlider').addEventListener('change', (e) => {
    sendCommand('temp', e.target.value);
});

// Toast notification
function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = `toast ${type} show`;
    
    setTimeout(() => {
        toast.classList.remove('show');
    }, 3000);
}

// Keyboard shortcuts
document.addEventListener('keydown', (e) => {
    if (e.key === '+' || e.key === '=') {
        increaseTemp();
    } else if (e.key === '-' || e.key === '_') {
        decreaseTemp();
    } else if (e.key === 'p' || e.key === 'P') {
        // Toggle power (you can customize this)
        sendCommand('power', 'on');
    }
});

// Handle online/offline status
window.addEventListener('online', () => {
    showToast('Internet connection restored', 'success');
    checkStatus();
});

window.addEventListener('offline', () => {
    showToast('Internet connection lost', 'error');
});

// Auto-refresh learned commands every 30 seconds
setInterval(loadLearnedCommands, 30000);