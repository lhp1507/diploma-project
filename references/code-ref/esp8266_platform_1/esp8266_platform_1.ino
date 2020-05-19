#include <SerialCommand.h>



/*
 * Hive MQ
 * user: admin
 * pass: hivemq
 **
 * Thư viện PubSubClient tham khảo: https://github.com/knolleary/pubsubclient/tree/master/src
 * Full API: https://pubsubclient.knolleary.net/
*/

/*
 * Thư viện:
 */
//Webserver và config wifi cho nodeMCU:
#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
//
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

/*
 * Cấu hình WiFi cho nodeMCU kết nối
 */
//#define ssid "DIPLOMA"
//#define password "chickendinner"

/*
 * Cấu hình MQTT
 */
//Server MQTT
//#define mqtt_server "192.168.1.112" //Dau Tran
char mqtt_server[50];
const uint16_t mqtt_port = 1883;
//Các topic
#define mqtt_sub_reminds "Reminds"
#define mqtt_pub_topic "Topic"

//#define mqtt_user "datn"
//#define mqtt_password "datn"

/*
 * 
 */
WiFiClient espClient;
PubSubClient client(espClient);

/*
 * Cấu hình AP nhập wifi
 */
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

/*
 * cài đặt giao tiếp tx rx
 * giữa esp8266 và mega2560
 */
//const byte RX = D7; //RX trên nodeMCU
//const byte TX = D8; //TX trên nodeMCU
//SoftwareSerial Serial(RX, TX); //khai báo tên megaSerial sử dụng chân TX RX
//SerialCommand sCmd(megaSerial); //khai báo biến sử dụng thư viện Serial Command
SerialCommand sCmd;

/*
 * khai báo các biến sử dụng
 */
//b. Khác:
  String REMINDS;
  byte timeOut = 60;

void setup()
{
  /*
   * Mở cổng Serial
   */
  Wire.begin();
  Serial.begin(9600);
//  Serial.begin(115200);
//  Serial.setTimeout(timeOut);
//  megaSerial.begin(9600); //khởi tạo megaSerial để giao tiếp với mega
  /*
   * Một số hàm trong thư viện Serial Command:
   */
  sCmd.addCommand("TP", topic); //Khi có lệnh "TOPIC" thì sẽ thực hiện hàm topic()
//  sCmd.addCommand("G", alarmTime);
//  sCmd.addCommand("T", tempRead);
//  sCmd.addCommand("H", humRead);
  
  /*
   * WiFiManager Auto connect
   */
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset WiFi:
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
        DynamicJsonBuffer jsonBuffer0;
        JsonObject& json0 = jsonBuffer0.parseObject(buf.get());
        json0.printTo(Serial);
        if (json0.success())
        {
          Serial.println("\nparsed json");
          strcpy(mqtt_server, json0["mqtt_server"]);
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

  if (!wifiManager.autoConnect("AutoConnectAP"))
  {
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
  if (shouldSaveConfig)
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer00;
    JsonObject& json00 = jsonBuffer00.createObject();
    json00["mqtt_server"] = mqtt_server;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }
    json00.printTo(Serial);
    json00.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("\nlocal ip");
  Serial.println(WiFi.localIP());

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
  sCmd.readSerial();
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String pload;
  for (int i = 0; i < length; i++)
  {
    pload += (char)payload[i];
  }
  if (String(topic) == String(mqtt_sub_reminds))
  {
    if(pload == "Alarm")
    {
      REMINDS = pload;
      
      const size_t capacity = JSON_OBJECT_SIZE(1);
      DynamicJsonBuffer jsonBuffer1(capacity); //tạo Buffer json có khả năng chứa tối đa 200 ký tự
      JsonObject& root1  = jsonBuffer1.createObject(); //tạo một biến root mang kiểu json
      root1["reminds"] = REMINDS;

//      String message = "RM "+String(REMINDS);
//      Serial.print("\nmessage: ");
//      Serial.println(message);
      
      Serial.print("RM"); //gửi tên lệnh
      Serial.print('\r'); //gửi \r
      root1.printTo(Serial); //gửi chuỗi Json cho esp
      Serial.print('\r'); //gửi \r
  
//      Serial.println("\n-----\nin message ra Serial để debug nè:");
//      Serial.print("message: ");
////      root1.printTo(Serial);
//      Serial.println(message);
//      Serial.println("\n----");
    }
  }
  else Serial.println("Khong hop le\n");
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
//    if (client.connect("ESP8266Client"), mqtt_user, mqtt_password)
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_pub_topic, "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_sub_reminds);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(", try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
 * nhận dữ liệu từ mega2560 qua TX RX
 */
void topic()
{
  Serial.println("TOPIC");
    const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
    DynamicJsonBuffer jsonBuffer2(capacity);
    char *json2 = sCmd.next(); //Chỉ cần 1 dòng này để đọc tham số nhận được
    JsonObject& root2 = jsonBuffer2.parseObject(json2);
    if(!root2.success())
    {
      Serial.println("JSON parsing failed");
    }
    long g = root2["G"];
    int t = root2["T"];
    int h = root2["H"];

    String G = String(g);
    String dataG = String("G" + G);
    String T = String(t);
    String dataT = String("T" + T);
    String H = String(h);
    String dataH = String("H" + H);

    unsigned int lenG = dataG.length()+1;
    unsigned int lenT = dataT.length()+1;
    unsigned int lenH = dataH.length()+1;
    
    char bufferG[50];
    char bufferT[50];
    char bufferH[50];
    
    dataG.toCharArray(bufferG, lenG);
    dataT.toCharArray(bufferT, lenT);
    dataH.toCharArray(bufferH, lenH);

    client.publish(mqtt_pub_topic, bufferG);
    client.publish(mqtt_pub_topic, bufferT);
    client.publish(mqtt_pub_topic, bufferH);
    
    Serial.print("bufferG: ");
    Serial.println(bufferG);
    Serial.print("bufferT: ");
    Serial.println(bufferT);
    Serial.print("bufferH: ");
    Serial.println(bufferH);
}

//void alarmTime()
//{
//  Serial.println("G:");
//  char *arg;
//  arg = sCmd.next();
//  if(arg != NULL)
//  {
//    long g = atoi(arg);
//    String G = String(g);
//    String dataG = String("G" + G);
//    unsigned int lenG = dataG.length();
//    char bufferG[20];
//    dataG.toCharArray(bufferG, lenG);
//    
//    client.publish(mqtt_pub_topic, bufferG);
//    
//    Serial.print("G esp gửi nè: ");
//    Serial.println(g);
//    Serial.print("bufferG: ");
//    Serial.println(bufferG);
//  }
//  else
//  {
//    Serial.println("arg = NULL");
//  }
//}
//
//void tempRead()
//{
//  Serial.println("T:");
//  char *arg;
//  arg = sCmd.next();
//  if(arg != NULL)
//  {
//    int t = atoi(arg);
//    String T = String(t);
//    String dataT = String("T" + T);
//    unsigned int lenT = dataT.length();
//    char bufferT[10];
//    dataT.toCharArray(bufferT, lenT);
//    
//    client.publish(mqtt_pub_topic, bufferT);
//    
//    Serial.print("T esp gửi nè: ");
//    Serial.println(t);
//    Serial.print("bufferT: ");
//    Serial.println(bufferT);
//  }
//  else
//  {
//    Serial.println("arg = NULL");
//  }
//}
//
//void humRead()
//{
//  Serial.println("H:");
//  char *arg;
//  arg = sCmd.next();
//  if(arg != NULL)
//  {
//    int h = atoi(arg);
//    String H = String(h);
//    String dataH = String("H" + H);
//    unsigned int lenH = dataH.length();
//    char bufferH[10];
//    dataH.toCharArray(bufferH, lenH);
//    
//    client.publish(mqtt_pub_topic, bufferH);
//    
//    Serial.print("H esp gửi nè: ");
//    Serial.println(h);
//    Serial.print("bufferH: ");
//    Serial.println(bufferH);
//  }
//  else
//  {
//    Serial.println("arg = NULL");
//  }
//}
