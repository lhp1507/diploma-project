/*
  It connects to a MQTT Broker then:
  - Subscribes to the topic "iot/light", printing out any messages
    it receives.
  - If the first character of the topic "iot/light" is '1', switch ON the ESP Led,
    else if '0' switch it off.
  - Library: https://github.com/knolleary/pubsubclient/
  - Documentation: https://pubsubclient.knolleary.net/api.html#connect1
*/
#include <WiFi.h>
#include <PubSubClient.h>
// Update these with values suitable for your network.
#define ssid "@__lhp1507"
#define password "HelloWorld"
//#define ssid "Tho Con"
//#define password "H0976929424"
#define mqtt_server "10.10.41.64"
#define mqtt_topic_pub "Dau/Light"
#define mqtt_topic_sub "Dau/Light"
//#define mqtt_user "client2"
//#define mqtt_password "123456"

const uint16_t mqtt_port = 1883;
const int LED = LED_BUILTIN;
const int BUTTON = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(LED, LOW);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);             //cau hinh
  client.setCallback(callback);                               //set hàm callback de nhan du lieu 

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
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//LED ON
//LED OFF
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
 //char p = (char)payload[0];
  //Serial.print((char)payload[0]);
  String p = "";
  for(int i=0; i<length;i++)
  {
    p = p +(char)payload[i];
    }
  Serial.println(p);
  if (p == "LEDON")
  {
    digitalWrite(LED, HIGH);
    Serial.println(" Turn On LED! " );
  }
  // if MQTT comes a 1, turn on LED on pin D2
  else if (p == "LEDOFF")
  {
    digitalWrite(LED, LOW);
    Serial.println(" Turn Off LED! " );
  }
  else Serial.println(" Khong hop le");
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic_pub, "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {                 //Neu chua ket noi thi goi lai ham reconnect
    reconnect();
  }
  client.loop();                             //Giu cho client luon ket noi de luon subcrib
  if (digitalRead(BUTTON) == 0) {
    while (digitalRead(BUTTON) == 0);
    delay(50);
    if (digitalRead(LED) != 0) {
      digitalWrite(LED, LOW);
      Serial.println("LED OFF");
       client.publish(mqtt_topic_pub, "0",1);
    }
    else
    {
      digitalWrite(LED, HIGH);
      Serial.println("LED ON");
       client.publish(mqtt_topic_pub, "1",1);
    }
  }
}
