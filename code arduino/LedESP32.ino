#include <WiFi.h>          
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi 
const char* ssid = "admin"; // Wifi SSID
const char* password = "ChoQRxemPassCaiLol"; // Wifi Password

// MQTT Broker
const char* mqtt_server = "192.168.126.169";
unsigned int mqtt_port = 1883; //SSL 8883 NoneSSL 1883
const char* username = "master:mqttuser"; // Service User Realm:Serviceuser
const char* mqttpass = "jrEeOQJfUCuIPOUZXajr9Iz8p3udYYw8"; // Service User Secret
const char* ClientID = "client123"; 
const char* lastwill = "master/client123/writeattributevalue/writeAttribute/3dmEu8zvK3Eseb0C0fzNWd";
const char* lastwillmsg = "0";

// Subscribing Topic
const char *topic = "master/client123/attribute/onOff/5a0O9cHSR4bYLfrvmqWiqB";

// LED
const int ledPin = 5;

int ledState = LOW;

WiFiClient askClient; // SSL Client
PubSubClient client(askClient);

void setup() {
  Serial.begin(115200);
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(10); // Delay nhỏ để tránh xử lý quá nhanh
}

// MQTT callback
void callback(char* receivedTopic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(receivedTopic);
  
  // Kiểm tra xem topic nhận được có khớp với topic mong muốn
  if (String(receivedTopic) == topic) {
    payload[length] = '\0'; // Đảm bảo payload là một chuỗi kết thúc đúng
    Serial.print("Message: ");
    Serial.println((char*)payload);

    // Phân tích JSON từ payload
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);

    // Kiểm tra lỗi khi phân tích JSON
    if (!error) {
      // Trích xuất giá trị của trường "value"
      bool value = doc["value"];
      Serial.print("Parsed value: ");
      Serial.println(value);

      // Kiểm tra giá trị "true" hoặc "false" và điều khiển LED
      if (value) {
        digitalWrite(ledPin, HIGH);
        ledState = HIGH;
        Serial.println("LED ON");
      } else {
        digitalWrite(ledPin, LOW);
        ledState = LOW;
        Serial.println("LED OFF");
      }
    } else {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
  }
}

// MQTT reconnect
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(ClientID, username, mqttpass, lastwill, 1, 1, lastwillmsg)) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
