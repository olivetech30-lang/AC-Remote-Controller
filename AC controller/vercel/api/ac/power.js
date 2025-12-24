// api/ac/power.js
export default async (req, res) => {
  if (req.method !== 'POST') return res.status(405).end();
  
  const { on } = req.body;
  // In real app: save to Supabase/DynamoDB
  // For demo: store in memory (resets on cold start)
  global.acState = global.acState || { command: null, executed: false };
  global.acState.command = on ? "power_on" : "power_off";
  global.acState.executed = false;
  
  res.json({ status: "queued" });
};