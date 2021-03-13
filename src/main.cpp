#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <constants.h>
#include <sensors.h>
#include <settings.h>

const char *mqttServer;
const char *mqttPassword;
const char *mqttClientId;

Settings settings;
WiFiClient espClient;

PubSubClient client(espClient);

typedef struct currentReading {
  const char *channel;
  float reading;
} CurrentReading;

QueueHandle_t currentQueue;

WiFiManager wifiManager;
WiFiManagerParameter custom_mqtt_server_client_id("serverclientid",
                                                  "MQTT Client Id",
                                                  "servername", 40);

WiFiManagerParameter custom_mqtt_server_host("serverhost", "MQTT Server Host",
                                             "host", 40);

void saveConfigCallback() {
  Settings settings;

  strcpy(settings.client_id, custom_mqtt_server_client_id.getValue());
  strcpy(settings.host, custom_mqtt_server_host.getValue());

  Serial.println("SETTING SETTINGS");
  Serial.print("Client Id: ");
  Serial.println(settings.client_id);

  Serial.print("Host: ");
  Serial.println(settings.host);

  setSettings(settings);
}

void clearWiFiSettings(void *pvParameters) {
  // Loop for 5 seconds before resetting. Because of the circuits physical
  // proximity to the circuit box
  for (int i = 0; i < 5; i++) {
    if (digitalRead(RESET_PIN) == LOW) {
      vTaskDelete(NULL);
      return;
    }
    vTaskDelay(1000);
  }

  Serial.println("Resetting WiFi");
  wifiManager.resetSettings();
  ESP.restart();
  vTaskDelete(NULL);
}

void IRAM_ATTR handleResetButtonPress() {
  xTaskCreate(clearWiFiSettings, "Clear WiFi Settings", 2056, NULL, 1, NULL);
}

void fetchCurrent(void *pvParameters) {
  for (;;) {
    vTaskDelay(1000);

    CurrentReading reading1;
    reading1.reading = readCurrentSensor(1);
    reading1.channel = "current_channel_1";

    xQueueSend(currentQueue, &reading1, 0);
    vTaskDelay(1000);

    CurrentReading reading2;
    reading2.reading = readCurrentSensor(2);
    reading2.channel = "current_channel_2";

    xQueueSend(currentQueue, &reading2, 0);
  }
}

void publishData(void *pvParameters) {
  for (;;) {
    CurrentReading reading;
    xQueueReceive(currentQueue, &reading, portMAX_DELAY);

    DynamicJsonDocument doc(1024);

    doc["sensor_id"] = reading.channel;
    doc["value"] = reading.reading;
    doc["unit"] = "A";

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(currentMQTTTopic, buffer, n);
  }
}

void setupTasks() {
  // setup queues
  currentQueue = xQueueCreate(1, sizeof(CurrentReading));

  // setup tasks
  xTaskCreate(fetchCurrent, "Fetch Task", 2056, NULL, 1, NULL);
  xTaskCreate(publishData, "Publish Data", 2056, NULL, 1, NULL);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);

  wifiManager.addParameter(&custom_mqtt_server_host);
  wifiManager.addParameter(&custom_mqtt_server_client_id);

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  bool res;
  res = wifiManager.autoConnect("Current Sensor Setup",
                                "current-sensor");  // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("WiFi connected");
  }
}

void connectMQTT(Settings settings) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(&settings.client_id[0])) {
      Serial.println("connected!");
      // TODO: add subscriptions here for commands
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);
  EEPROM.begin(256);
  Serial.begin(115200);

  // setup interupts
  pinMode(RESET_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(RESET_PIN), handleResetButtonPress,
                  RISING);

  // Setup current sensor
  setupCurrentSensor();

  // setup WiFi
  setupWiFi();

  settings = getSettings();

  Serial.print("Client Id: ");
  Serial.println(settings.client_id);

  Serial.print("Host: ");
  Serial.println(settings.host);

  client.setServer(settings.host, 1883);

  setupTasks();
}

void loop() {
  if (!client.connected()) {
    connectMQTT(settings);
  }
}