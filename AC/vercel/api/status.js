// vercel/api/status.js
export default async (req, res) => {
  const ESP32_IP = '192.168.18.230'; // Replace with your ESP32 IP
  try {
    const espRes = await fetch(`http://${ESP32_IP}/status`);
    const data = await espRes.json();
    res.status(200).json(data);
  } catch (e) {
    res.status(500).json({ error: 'ESP32 unreachable' });
  }
};