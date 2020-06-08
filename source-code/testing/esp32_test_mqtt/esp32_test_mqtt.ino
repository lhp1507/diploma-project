/*
 * NOTE:
 * Code này đã chạy được
 * Kết nối vào wifi: OK
 * Kết nối mqtt: OK
 * nhận callback: OK
 * reconnect: OK
 * điều kiện loop: OK
 */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WebServer.h>

#define mqtt_sub_reminds "Reminds"
#define mqtt_pub_topic "Topic"

const char* mqtt_server = "192.168.0.142";
const uint16_t mqtt_port = 1883;

// SSID and PW for your Router
const char* Router_SSID = "@__lhp1507";
const char* Router_PASS = "HelloWorld";

WiFiClient espClient;
PubSubClient client(espClient); //lib required for mqtt

static volatile uint8_t REMINDS = 0;

void connectmqtt()
{
  client.connect("ESP32_ClientID");  // ESP will connect to mqtt broker with clientID
  {
    Serial.print("connected to MQTT at ");
    Serial.println(mqtt_server);
    client.subscribe(mqtt_sub_reminds); //topic=Demo
    client.publish(mqtt_pub_topic,  "connected to MQTT");

    if (!client.connected())
    {
      reconnect();
    }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect("ESP32_ClientID"))
    {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
      client.publish(mqtt_pub_topic, "ESP32_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_sub_reminds);
    }
    else
    {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("DATN!"));
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(Router_SSID);

  WiFi.begin(Router_SSID, Router_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
//  delay(1000);
  connectmqtt();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  if(REMINDS == 1)
  {
    Serial.println("ALARM.....");
    delay(1000);
    REMINDS = 0;
  }
  
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print(("Message arrived in topic ["));
  Serial.print(topic);
  Serial.print(("] "));
  String pload;
  for (int i = 0; i < length; i++)
  {
    pload += (char)payload[i];
  }
  Serial.print("\nMessage: ");
  Serial.println(pload);
  if (String(topic) == String(mqtt_sub_reminds))
  {
    if(pload == "Alarm")
    {
      REMINDS = 1;
      Serial.print("payload in 'Alarm' - REMINDS: ");
      Serial.print(REMINDS);
      Serial.println();
    }
  }
}
