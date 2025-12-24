// api/ac/ack.js
export default (req, res) => {
  if (global.acState) {
    global.acState.executed = true;
  }
  res.json({ status: "acknowledged" });
};