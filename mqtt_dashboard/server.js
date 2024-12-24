const mqtt = require('mqtt');
const express = require('express');
const http = require('http');
const socketIo = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Serve static files for the dashboard
app.use(express.static('public'));

// Connect to MQTT broker
const mqttClient = mqtt.connect('mqtt://broker.hivemq.com');

mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  mqttClient.subscribe('esp32/temperature');
  mqttClient.subscribe('esp32/humidity');
});

// Forward MQTT messages to the dashboard
mqttClient.on('message', (topic, message) => {
  console.log(`${topic}: ${message}`);
  io.emit('mqtt', { topic, message: message.toString() });
});

// Start the server
const PORT = 3001;
server.listen(PORT, () => console.log(`Server running on http://localhost:${PORT}`));
