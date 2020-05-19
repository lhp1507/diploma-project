/*
 * Hive MQ
 * user: admin
 * pass: hivemq
 **
 * Thư viện PubSubClient tham khảo: https://github.com/knolleary/pubsubclient/tree/master/src
 * Full API: https://pubsubclient.knolleary.net/
*/

#include <FS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>

// Update these with values suitable for your network.
//#define ssid "HACKED"
//#define password "Tapit168"
//#define mqtt_server "192.168.137.226"

#define mqtt_topic_sub_light1 "light1/cmd"
#define mqtt_topic_sub_light2 "light2/cmd"
#define mqtt_topic_pub_light1 "light1/state"
#define mqtt_topic_pub_light2 "light2/state"
#define mqtt_topic_pub_temp "temp/state"
#define mqtt_topic_pub_hum "hum/state"

//#define mqtt_user "admin"
//#define mqtt_password "hivemq"

char mqtt_server[40];
const uint16_t mqtt_port = 1883;  //Port chuan cua MQTT

#define interruptButton1Pin D2
#define interruptButton2Pin D3
#define light1Pin D7
#define light2Pin D8

const int DHTTYPE = DHT11;
const int DHTPIN = D5;
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

//long lastMsg = 0;
//char msg[50];
//int value = 0;
unsigned long readTime;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void ICACHE_RAM_ATTR pubLight1(void)
{
  if (digitalRead(interruptButton1Pin) == LOW)
  {
    while (digitalRead(interruptButton1Pin) == LOW);
    if (digitalRead(light1Pin) == LOW)
    {
      digitalWrite(light1Pin, HIGH);
      client.publish(mqtt_topic_pub_light1, "ON");
    }
    else if (digitalRead(light1Pin) == HIGH)
    {
      digitalWrite(light1Pin, LOW);
      client.publish(mqtt_topic_pub_light1, "OFF");
    }
    Serial.println("interrupt");
  }
}

void ICACHE_RAM_ATTR pubLight2()
{
  if (digitalRead(interruptButton2Pin) == LOW)
  {
    while (digitalRead(interruptButton2Pin) == LOW);
    if (digitalRead(light2Pin) == LOW)
    {
      digitalWrite(light2Pin, HIGH);
      client.publish(mqtt_topic_pub_light2, "ON");
    }
    else if (digitalRead(light2Pin) == HIGH)
    {
      digitalWrite(light2Pin, LOW);
      client.publish(mqtt_topic_pub_light2, "OFF");
    }
    Serial.println("interrupt");
  }
}

void setup()
{
  Serial.begin(115200);
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
//  wifiManager.resetSettings();
  //read configuration from FS json
  
  Serial.println("mounting FS...");
  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success())
        {
          Serial.println("\nparsed json");
          strcpy(mqtt_server, json["mqtt_server"]);
        }
        else
        {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);

  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

   //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  
  dht.begin();
  readTime = 0;
  
  pinMode(interruptButton1Pin, INPUT_PULLUP);
  pinMode(interruptButton2Pin, INPUT_PULLUP);
  pinMode(light1Pin, OUTPUT);
  pinMode(light2Pin, OUTPUT);
  digitalWrite(light1Pin, LOW);
  digitalWrite(light2Pin, LOW);

  attachInterrupt(digitalPinToInterrupt(interruptButton1Pin), pubLight1, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptButton2Pin), pubLight2, FALLING);
 
//  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  //check if 5 seconds has elapsed since the last time we read the sensors. 
  if (millis() - readTime >= 60000)
  {
    dhtRead();
  }
}
/*
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
*/
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String palo;
  for (int i = 0; i < length; i++)
  {
    palo += (char)payload[i];
  }
  if (String(topic) == String(mqtt_topic_sub_light1))
  {
    if (palo == "ON") 
    {
      Serial.print(palo);
      digitalWrite(light1Pin, HIGH); 
      Serial.println(" Turn On LED! " );
    }
    else if (palo == "OFF")
    {
      Serial.print(palo);
      digitalWrite(light1Pin, LOW); 
      Serial.println(" Turn Off LED! " );
    }
  }
  else if (String(topic) == String(mqtt_topic_sub_light2))
  {
    if (palo == "ON") 
    {
      Serial.print(palo);
      digitalWrite(light2Pin, HIGH); 
      Serial.println(" Turn On LED! " );
    } 
    else if (palo == "OFF")
    {
      Serial.print(palo);
      digitalWrite(light2Pin, LOW); 
      Serial.println(" Turn Off LED! " );
    }
  }
  else Serial.println(" Khong hop le");
  Serial.println();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic_pub_light1, "ESP_reconnected");
      client.publish(mqtt_topic_pub_light2, "ESP_reconnected");
      client.publish(mqtt_topic_pub_temp, "ESP_reconnected");
      client.publish(mqtt_topic_pub_hum, "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub_light1);
      client.subscribe(mqtt_topic_sub_light2);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void dhtRead()
{
  readTime = millis();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  float hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temp = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  char buffer[10];
  dtostrf(temp, 0, 0, buffer);
  client.publish(mqtt_topic_pub_temp, buffer);
  //Serial.println(buffer);
  dtostrf(hum, 0, 0, buffer);
  client.publish(mqtt_topic_pub_hum, buffer);
  
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" *C ");
}
