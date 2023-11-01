const express = require('express');
const http = require('http');
const net = require('net');
const socket = require('socket.io');
const cors = require('cors');

const app = express();
const server = http.createServer(app);

// Enable CORS for all routes
app.use(cors());

const HOST = '192.168.1.84'; // Replace with the IP address of your server
const PORT = 4242; // Replace with the port you're using

const client = new net.Socket();
let receivedData = ''; // Variable to store received data

client.connect(PORT, HOST, () => {
    console.log('Connected to the server');
});

client.on('data', (data) => {
    // Data received from the server
    console.log(`Received data from server: ${data}`);
    receivedData = data.toString(); // Store received data
    sendDataToServer(); // Send a reply
});

client.on('close', () => {
    console.log('Connection closed');
});

client.on('error', (err) => {
    console.error('Error:', err);
});

function sendDataToServer() {
    // Prepare and send data to the server
    const data = Buffer.from('hello', 'utf8');
    client.write(data);
    console.log('send' + data)
}

// Serve HTML page
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

// Serve received data as an API endpoint
app.get('/api/received-data', (req, res) => {
    res.json({ message: receivedData });
});

const PORT_SERVER = 3000; // Change to your desired port
server.listen(PORT_SERVER, () => {
    console.log(`Server is running on port ${PORT_SERVER}`);
});