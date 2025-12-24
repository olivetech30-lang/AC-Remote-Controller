// api/ac/state.js
export default (req, res) => {
  // In real app: read from DB
  // For demo: simulate state
  const state = global.acState || { command: "none", executed: true };
  const isPowerOn = state.command === "power_on" && !state.executed;
  const temp = state.command?.startsWith("temp_") ? 
    parseInt(state.command.split("_")[1]) : 24;
  
  res.json({ 
    power: isPowerOn, 
    temp: temp 
  });
};