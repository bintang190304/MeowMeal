#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Stepper.h>

#define DHTPIN D4
#define IN1 D8
#define IN2 D7
#define IN3 D6
#define IN4 D5

// Define the number of steps per revolution
const int stepsPerRevolution = 2048;

Stepper stepper(stepsPerRevolution, IN4, IN2, IN3, IN1);

DHTesp dht;

const char* ssid = "Itu";
const char* password = "kagakada";
const char* mqtt_server = "broker.emqx.io";
const char* sensorDataTopic = "sensor/data";
const char* feedControlTopic = "feeding/data";

WiFiClient espClient;
PubSubClient client(espClient);
bool motorRunning = false;
unsigned long motorStartTime = 0;
const unsigned long motorRunTime = 20000; // 10 detik dalam milidetik

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  dht.setup(DHTPIN, DHTesp::DHT11);

  connectToWiFi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);

  stepper.setSpeed(15); // Set the speed of the stepper motor
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (motorRunning) {
    // Cek apakah motor sudah berjalan lebih dari 10 detik
    if (millis() - motorStartTime < motorRunTime) {
      // Step the motor continuously in the clockwise direction
      stepper.step(1);
    } else {
      motorRunning = false;
      stopMotor();
    }
  } else {
    float h = dht.getHumidity();
    float t = dht.getTemperature();
    
    String payload = "{\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}";
    client.publish(sensorDataTopic, payload.c_str());

    delay(2000); // Tunggu 2 detik sebelum pembacaan berikutnya
  }
}

void forwardMotor() {
  motorStartTime = millis(); // Catat waktu mulai motor berjalan
  motorRunning = true;
}

void stopMotor() {
  // No need to do anything specific to stop the motor; it will naturally stop
  // when motorRunning is set to false.
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  if (String(topic) == feedControlTopic) {
    if (message == "on") {
      forwardMotor();
    } else if (message == "off") {
      motorRunning = false;
      stopMotor();
    }
  }
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX); // Generate a random client ID
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(feedControlTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
