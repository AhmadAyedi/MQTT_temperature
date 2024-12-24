#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Wi-Fi credentials
const char* ssid = "TOPNET_21F8"; // Replace with your WiFi name
const char* password = "yp5qac98w8"; // Replace with your WiFi password

// MQTT server details
const char* mqtt_server = "broker.hivemq.com"; // Replace with your MQTT broker
const int mqtt_port = 1883;                    // MQTT port (default is 1883)
const char* mqtt_user = "";                    // MQTT username (if required)
const char* mqtt_password = "";                // MQTT password (if required)

// MQTT topics
const char* temp_topic = "esp32/temperature";
const char* hum_topic = "esp32/humidity";

// DHT sensor setup
#define DHTPIN 4 // Pin connected to DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

// Reconnection delay
unsigned long lastMsg = 0;
const long interval = 2000; // Publish every 2 seconds

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(WiFi.macAddress());
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Validate readings
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor! Skipping this reading.");
      return;
    }

    // Ignore unrealistic values
    if (temperature < -40 || temperature > 80 || humidity < 0 || humidity > 100) {
      Serial.println("Unrealistic values detected! Skipping this reading.");
      return;
    }

    // Print values to serial monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    // Publish values
    String tempPayload = String(temperature);
    String humPayload = String(humidity);

    if (client.publish(temp_topic, tempPayload.c_str())) {
      Serial.println("Temperature sent successfully!");
    } else {
      Serial.println("Failed to send temperature!");
    }

    if (client.publish(hum_topic, humPayload.c_str())) {
      Serial.println("Humidity sent successfully!");
    } else {
      Serial.println("Failed to send humidity!");
    }
  }
}
