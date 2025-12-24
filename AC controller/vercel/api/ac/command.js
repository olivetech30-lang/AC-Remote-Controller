// api/ac/command.js
export default (req, res) => {
  const state = global.acState || { command: "none", executed: true };
  res.json(state);
};