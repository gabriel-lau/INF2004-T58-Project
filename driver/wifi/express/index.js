const express = require("express");
const http = require("http");
const net = require("net");
const cors = require("cors");

const app = express();
const server = http.createServer(app);

// Run wuth node index.js 

// Enable CORS for all routes
app.use(
  cors({
    origin: "*",
  })
);

const HOST = "192.168.1.84"; // Replace with the IP address of your server
const PORT = 4242; // Replace with the port you're using

const client = new net.Socket();
let receivedData = ""; // Variable to store received data

client.connect(PORT, HOST, () => {
  console.log("Connected to the server");
  // Send data to the server
});

client.on("data", (data) => {
  // Data received from the server
  console.log(`Received data from server: ${data}`);
  receivedData = data.toString(); // Store received data
  sendDataToServer(receivedData);
});

client.on("close", () => {
  console.log("Connection closed");
});

client.on("error", (err) => {
  console.error("Error:", err);
});

function sendDataToServer(reply) {
  // Prepare and send data to the server
  const data = Buffer.from(reply, "utf8");
  client.write(reply);
  console.log("Sent data to server: " + reply);
}

// Serve HTML page
app.get("/", (req, res) => {
  res.sendFile(__dirname + "/index.html");
});

// Serve received data as an API endpoint
app.get("/api/received-data", (req, res) => {
  // Implement the logic to provide the received data here
  // For simplicity, we'll just return a static message
  res.json({ message: receivedData });
});

let PORT_SERVER = process.env.PORT || 3000; // Change to your desired port
server.listen(PORT_SERVER, () => {
  console.log(`Server is running on port ${PORT_SERVER}`);
});
