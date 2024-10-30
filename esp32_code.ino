
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char* ssid = "SSID";
const char* password = "pass";
const char* mqtt_server = "test.mosquitto.org";
float temperature;
float humidity;
char bufferTemp[5];
char bufferHum[5];
uint32_t prevMillis;


const char* SUBSCRIBE_LED = "edutic/dht11/led";
const char* PUBLISH_TEMP = "edutic/dht11/temperature";
const char* PUBLISH_HUMID = "edutic/dht11/humidity";
const char* PUBLISH_JSON = "edutic/dht11/json";


#define LED 2
#define DHTPIN 27
#define DHTTYPE DHT11

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan Masuk dari topic [");
  Serial.print(topic);
  Serial.print("] :");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    digitalWrite(LED, HIGH);
    Serial.println("LED ON");
  } else {
    digitalWrite(LED, LOW);
    Serial.println("LED OFF");
  }

}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected!");
      mqtt.subscribe(SUBSCRIBE_LED);
      Serial.println("Subscribe ke topic: " + String(SUBSCRIBE_LED));
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println("Reconnect dalam 5 detik");
      delay(5000);
    }
  }
}

void publish_data() {
  sprintf(bufferTemp, "%.1f", temperature);
  mqtt.publish(PUBLISH_TEMP, bufferTemp);

  sprintf(bufferHum, "%.1f", humidity);
  mqtt.publish(PUBLISH_HUMID, bufferHum);
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
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);
}

void loop() {
  if (millis() - prevMillis >= 60000) {
    if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();


    publish_json();
    //publish_data();
    prevMillis = millis();
  }
}
