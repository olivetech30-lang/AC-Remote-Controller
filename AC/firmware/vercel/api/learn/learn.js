// Note: Vercel supports nested folders!
export default async (req, res) => {
  const ESP32_IP = '192.168.18.230';
  const url = `http://${ESP32_IP}/learn/poweron`; // Fixed path
  try {
    const espResponse = await fetch(url);
    const data = await espResponse.text();
    res.status(200).send(data);
  } catch (e) {
    res.status(500).json({ error: 'ESP32 unreachable' });
  }
};