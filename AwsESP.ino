#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> //https://pubsubclient.knolleary.net/
#include <WiFiUdp.h>
#include "certs.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


const char* ssid = "SSID"; // Your SSID
const char* password = "PASSWORD"; // Wifi password
const char* mqtt_server = "GATEWAY.iot.eu-west-1.amazonaws.com"; // your aws IoT Gateway

//Rele
byte relON[] = {0xA0, 0x01, 0x01, 0xA2};  //Hex command to send to serial for open relay
byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1}; //Hex command to send to serial for close relay

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0';

  if (strcmp((char *)payload, "{ \"message\": \"ON\" }") == 0) {
    Serial.write(relON, sizeof(relON));
    Serial.println();
  } else {
    Serial.write(relOFF, sizeof(relOFF));
    Serial.println();
  }
}

void reconnect() {
    timeClient.begin();
    while(!timeClient.update()){
     timeClient.forceUpdate();
      }

  wifiClient.setX509Time(timeClient.getEpochTime());
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      /* subscribe toyour Mqtt  */
      mqttClient.subscribe("somethingmeaningfull/1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      wifiClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_wifiClient() {
  wifiClient.setCertificate(cert_der, cert_der_len);
  wifiClient.setPrivateKey(private_der, private_der_len);
  wifiClient.setCACert(cacert_der, cacert_der_len);
}

void setup_mqttClient() {
  mqttClient.setServer(mqtt_server, 8883);
  mqttClient.setCallback(callback);
}

void setup() {
  Serial.begin (9600);

  setup_wifi();

  setup_wifiClient();

  setup_mqttClient();
}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}
