#include <WiFi.h>          
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>  // Thêm thư viện DHT
#include <LiquidCrystal_I2C.h>

// WiFi 
const char* ssid = "admin"; // Wifi SSID
const char* password = "ChoQRxemPass"; // Wifi Password

// MQTT Broker
const char* mqtt_server = "192.168.126.169";
unsigned int mqtt_port = 1883; //SSL 8883 NoneSSL 1883
const char* username = "master:mqttuser"; // Service User Realm:Serviceuser
const char* mqttpass = "jrEeOQJfUCuIPOUZXajr9Iz8p3udYYw8"; // Service User Secret
const char* ClientID = "client123"; 
// LastWill
const char* lastwill = "master/client123/writeattributevalue/writeAttribute/3dmEu8zvK3Eseb0C0fzNWd";
const char* lastwillmsg = "0";

// Subscribing Topic
const char *topic = "master/client123/attribute/subscribeAttribute/3dmEu8zvK3Eseb0C0fzNWd";

// Localhost Certificate
// const char* local_root_ca 

// DHT Sensor Settings
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); 

// Objects
WiFiClient askClient; // SSL Client
PubSubClient client(askClient);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(WiFi.localIP());

  // askClient.setCACert(local_root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  dht.begin();

  // Khởi tạo màn hình LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("DFox...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(3, 0);
  lcd.print(WiFi.localIP());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Đọc dữ liệu từ cảm biến DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra xem dữ liệu có hợp lệ không
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Chuyển đổi dữ liệu thành chuỗi để publish lên MQTT
  String temp = String(t);
  String hum = String(h);

  // Publish dữ liệu lên các topic MQTT
  client.publish("master/client123/writeattributevalue/temperature/3dmEu8zvK3Eseb0C0fzNWd", temp.c_str());
  client.publish("master/client123/writeattributevalue/humidity/3dmEu8zvK3Eseb0C0fzNWd", hum.c_str());
  Serial.print("Temp: ");
  Serial.println(temp.c_str());
  Serial.print("Hum: ");
  Serial.println(hum.c_str());
  Serial.println("- - - - -");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(hum);
  lcd.print("%");

  delay(10000); // Delay 10 giây trước khi gửi dữ liệu tiếp theo
}


// MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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