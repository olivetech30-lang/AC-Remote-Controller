// api/learn.js - Learning mode endpoint
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
  const { cmd } = req.query;

  if (!cmd) {
    return res.status(400).json({ 
      error: 'Missing cmd parameter',
      usage: '/api/learn?cmd=POWER_ON',
      examples: ['POWER_ON', 'POWER_OFF', 'TEMP_24']
    });
  }

  try {
    const espUrl = `http://${ESP32_IP}/learn?cmd=${encodeURIComponent(cmd)}`;
    const response = await fetch(espUrl, { 
      method: 'GET',
      headers: { 'Content-Type': 'application/json' },
      timeout: 10000 // Learning might take longer
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