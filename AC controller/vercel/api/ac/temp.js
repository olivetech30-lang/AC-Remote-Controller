// api/ac/temp.js
export default async (req, res) => {
  if (req.method !== 'POST') return res.status(405).end();
  
  const { temp } = req.body;
  if (temp < 16 || temp > 30) {
    return res.status(400).json({ error: "Temp must be 16-30" });
  }
  
  global.acState = global.acState || { command: null, executed: false };
  global.acState.command = `temp_${temp}`;
  global.acState.executed = false;
  
  res.json({ status: "queued" });
};