
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

//setup pin DHT
#define DHTPIN 14
#define DHTTYPE DHT11

//Setup WiFi
const char* ssid = "SSID";
const char* password = "pass";

//Setup MQTT
const char* mqtt_server = "broker.emqx.io";
const char* PUBLISH_JSON = "edutic/dht11";
int intervalPengiriman = 5; // dalam Detik

float temperature;
float humidity;
char bufferTemp[5];
char bufferHum[5];
uint32_t prevMillis;


WiFiClient espClient;
PubSubClient mqtt(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Terhubung");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println("Reconnect dalam 5 detik");
      delay(5000);
    }
  }
}

void publish_json() {

  char json[256];
  JsonDocument doc;

  sprintf(bufferTemp, "%.1f", temperature);
  sprintf(bufferHum, "%.1f", humidity);

  doc["temperature"] = bufferTemp;
  doc["humidity"] = bufferHum;

  serializeJson(doc, json);
  mqtt.publish(PUBLISH_JSON, json);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  mqtt.setServer(mqtt_server, 1883);
}

void loop() {
  if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();
  
  if (millis() - prevMillis >= intervalPengiriman*1000) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    
    publish_json();
    prevMillis = millis();
  }
}
