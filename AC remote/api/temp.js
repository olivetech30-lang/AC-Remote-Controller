// api/temp.js - Temperature control endpoint
export default async function handler(req, res) {
  // Enable CORS
  res.setHeader('Access-Control-Allow-Credentials', true);
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET,OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  // Handle preflight
  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }

  // Get ESP32 IP from environment variable or use default
  const ESP32_IP = process.env.ESP32_IP || '192.168.18.230';
  const { temp } = req.query;

  if (!temp) {
    return res.status(400).json({ 
      error: 'Missing temp parameter',
      usage: '/api/temp?temp=24 (range: 16-30)'
    });
  }

  // Validate temperature range
  const temperature = parseInt(temp);
  if (temperature < 16 || temperature > 30) {
    return res.status(400).json({ 
      error: 'Temperature out of range',
      message: 'Temperature must be between 16 and 30'
    });
  }

  try {
    const espUrl = `http://${ESP32_IP}/temp?temp=${temp}`;
    const response = await fetch(espUrl, { 
      method: 'GET',
      headers: { 'Content-Type': 'application/json' },
      timeout: 5000
    });
    
    const data = await response.json();
    return res.status(200).json(data);
  } catch (error) {
    console.error('ESP32 Error:', error);
    return res.status(500).json({ 
      error: 'ESP32 unreachable',
      message: error.message,
      esp32_ip: ESP32_IP
    });
  }
}