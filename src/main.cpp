#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WifiUtil.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SLEEP_DELAY_IN_SECONDS  300
#define ONE_WIRE_BUS            D4      // DS18B20 pin
#define FORCE_DEEPSLEEP

const char* mqtt_server = "192.168.1.109";
WifiUtil wifiUtil ("SSID", "PASS");
WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

char temperatureString[6];
char msg[50];

void setup() {
  Serial.begin(9600);
  wifiUtil.setup_wifi();
  client.setServer(mqtt_server, 1883);

  DS18B20.begin();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp8266-01", "/lwt", 0, false, "offline")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void deepSleep() {
  #ifdef FORCE_DEEPSLEEP
    Serial.println("Entering deep sleep");
    ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
    delay(500);
  #endif
}

float getTemperature() {
  Serial.println("Requesting DS18B20 temperature...");
  float temp;
  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

void measure() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temperature = getTemperature();
  dtostrf(temperature, 2, 2, temperatureString);

  Serial.print("temperature is: ");
  Serial.println(temperatureString);

  sprintf(msg, "field1=%s&field2=%s", temperatureString, "0");

  Serial.print("Publish message over MQTT: ");
  Serial.println(msg);

  int result = client.publish("wohnzimmer/temperature_humidity", msg, 0);
  Serial.print("Publish success: ");
  Serial.println(result);
}

void loop() {
  measure();

  delay(100);

  Serial.println("Closing MQTT connection...");
  client.disconnect();
  Serial.println("Closing WiFi connection...");
  WiFi.disconnect();

  deepSleep();
}
