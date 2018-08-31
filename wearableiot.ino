#define ARDUINO_ARCH_ESP8266
#define DEBUG_WIFI_LOCATION
//#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WifiLocation.h>
#include <time.h>


// --------------------------------------------------------------------------------------------
//        UPDATE CONFIGURATION TO MATCH YOUR ENVIRONMENT
// --------------------------------------------------------------------------------------------

// Watson IoT connection details
//5omkne: organization id
//ESP8266: device type
//device01: device id
//espdevice: auth token


#define MQTT_HOST "5omkne.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:5omkne:ESP8266:device01"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "espdevice"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_CMD "iot-2/cmd/display/fmt/json"

// Add WiFi connection information
char ssid[] = "VodafoneMobileWiFi-DE6060";  // your network SSID (name)
char pass[] = "5276903048";  // your network password
const char* googleApiKey = "KJzaSyDbGHQERZbojp-ic_OonHlxkWVKEuz-gqH";

//age calculation
int timezone = 2;
int dst = 0;

//Location Object
WifiLocation location(googleApiKey);
location_t aLocation;

// variables to hold data
#define MSG_SIZE 250
StaticJsonBuffer<MSG_SIZE> jsonBuffer;
JsonObject& payload = jsonBuffer.createObject();
static char msg[MSG_SIZE];

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
}

void setup() {
  // put your setup code here, to run once:
// Start serial console
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println();
  Serial.println("ESP8266 Sensor Application");

  // Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");  
  //get location
  aLocation = location.getGeoFromWiFi();
  Serial.println("Location request data");
  Serial.println(location.getSurroundingWiFiJson());
  Serial.println("Latitude: " + String(aLocation.lat, 7));
  Serial.println("Longitude: " + String(aLocation.lon, 7));
  Serial.println("Accuracy: " + String(aLocation.accuracy));

  //sync time with server
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  // Connect to MQTT - IBM Watson IoT Platform
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_CMD);

  } else {
    Serial.println("MQTT Failed to connect!");
//    ESP.reset();
  }
}

void loop() {
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_CMD);
      mqtt.loop();
    } else {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }
  Serial.println(location.getSurroundingWiFiJson());
  Serial.println("Latitude: " + String(aLocation.lat, 7));
  Serial.println("Longitude: " + String(aLocation.lon, 7));
  Serial.println("Accuracy: " + String(aLocation.accuracy));
  // put your main code here, to run repeatedly:
  payload["type"] = "blind"; 
  payload["Lat"] = String(aLocation.lat, 7);//"29.958579";//
  payload["Long"] = String(aLocation.lon, 7);//"31.308020";//
//  payload["birth"] = "14011980";
  payload.printTo(msg, MSG_SIZE);
  Serial.println(msg);

  if (!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");
    }
  // Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < 10; i++) {
    mqtt.loop();
    delay(5000);
  }
}
