/*
 * NOTE:
 * 
 * 
 */
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ESP_WiFiManager.h>              //https://github.com/khoih-prog/ESP_WiFiManager

#include <WiFi.h>

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Tone32.h>
#include <DHT.h>
/*
 * cấu hình dht11 
 */
#define DHTTYPE   DHT11  //loại dht sử dụng
#define DHTPIN    15       //chân nhận dữ liệu
DHT dht(DHTPIN, DHTTYPE);   //cài đặt sử dụng DHT

#define BUZZER_PIN      14
#define BUZZER_CHANNEL  0
#define B_SET     36    //nút set menu
#define B_PLUS    39    //nút CỘNG
#define B_MINUS   35    //nút TRỪ
#define B_ONOFF   34    //nút bật/tắt báo thức
//#define INPUT_C   14    //74hc138 - C
//#define INPUT_B   18    //74hc138 - B
//#define INPUT_A   13    //74hc138 - A

#define mode_ON   1
#define mode_OFF  0

#define ESP_getChipId() ((uint32_t)ESP.getEfuseMac())

#define MQTT_SERVER_MAX_LEN   40
#define MQTT_SERVER_PORT_LEN  6

#define mqtt_sub_reminds "Reminds"
#define mqtt_pub_topic "Topic"

/*
 * icon:
 */
byte temperature[8] = //biểu tượng nhiệt độ 
{
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110
};
byte humidity[8] = //biểu tượng độ ẩm
{
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B01110,
};


/*
 * BUTTON config
 */
// riêng 
int buttonStateSET;             // the current reading from the input pin
int lastButtonStateSET = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTimeSET = 0;  // the last time the output pin was toggled
unsigned long debounceDelaySET = 50;    // the debounce time; increase if the output flickers

int buttonStatePL;             // the current reading from the input pin
int lastButtonStatePL = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTimePL = 0;  // the last time the output pin was toggled
unsigned long debounceDelayPL = 50;    // the debounce time; increase if the output flickers

int buttonStateMN;             // the current reading from the input pin
int lastButtonStateMN = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTimeMN = 0;  // the last time the output pin was toggled
unsigned long debounceDelayMN = 50;    // the debounce time; increase if the output flickers

int buttonStateOO;             // the current reading from the input pin
int lastButtonStateOO = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTimeOO = 0;  // the last time the output pin was toggled
unsigned long debounceDelayOO = 50;    // the debounce time; increase if the output flickers
//END BUTTON config

char configFileName[] = "/config.json";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// SSID and PW for Config Portal
String AP_SSID;
String AP_PASS;

char mqtt_server [MQTT_SERVER_MAX_LEN];
char mqtt_port   [MQTT_SERVER_PORT_LEN] = "1883";

WiFiClient espClient;
PubSubClient client(espClient); //lib required for mqtt

/*
 * cấu hình LCD: SDA:21; SCL:22
 */
LiquidCrystal_I2C lcd(0x27,16,2); //khai báo địa chỉ và kích thước lcd 16x2

/*
 * khai báo các biến sử dụng
 */
//a. khai báo menu là biến volatile vì có thay đổi ở CT ngắt
  static volatile uint8_t menu;

//c. báo thức:
  static volatile unsigned long alarm_hour = 0, alarm_minute = 0; //Giữ giờ báo thức mặc định hiện tại là 00:00
  static volatile unsigned long alarm_sec;
  String sec_to_mqtt;

  static volatile uint16_t alarm_left; //Số báo thức còn lại
  
  static volatile byte mode_ALARM = mode_ON;
  /*
   * REMINDS:
   * 1 - "Alarm" nhận từ mqtt
   * 2 - tìm hộp thuốc
   */
  static volatile uint8_t REMINDS = 0;

  unsigned long lastSend = 0;
  unsigned long lastRead = 0;
  unsigned long lastRing = 0;
  unsigned long lastDisplay = 0;
  unsigned long readInterval = 10000; //10s
  unsigned long sendInterval = 10000; //10s
  unsigned long displayGap = 150;
  unsigned long buzzerGap = 500;

  char bufferG[20];
  char bufferT[5];
  char bufferH[5];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback(void)
{
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

bool loadSPIFFSConfigFile(void)
{
  //clean FS, for testing
//  SPIFFS.format();

  //read configuration from FS json
  Serial.println(F("Mounting FS..."));

  if (SPIFFS.begin())
  {
    Serial.println(F("Mounted file system"));

    if (SPIFFS.exists(configFileName))
    {
      //file exists, reading and loading
      Serial.println(F("Reading config file"));
      File configFile = SPIFFS.open(configFileName, "r");

      if (configFile)
      {
        Serial.print(F("Opened config file, size = "));
        size_t configFileSize = configFile.size();
        Serial.println(configFileSize);

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[configFileSize + 1]);

        configFile.readBytes(buf.get(), configFileSize);

        Serial.print(F("\nJSON parseObject() result : "));

        DynamicJsonBuffer jsonBuffer;
        // Parse JSON string
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        // Test if parsing succeeds.

        if (json.success())
        {
          Serial.println(F("\nOK -- parsed json"));

          if (json["mqtt_server"])
            strncpy(mqtt_server, json["mqtt_server"], sizeof(mqtt_server));

          if (json["mqtt_port"])  
            strncpy(mqtt_port,   json["mqtt_port"], sizeof(mqtt_port));
        }
        else
        {
          Serial.println(F("failed to load json config"));
          return false;
        }
        //json.printTo(Serial);
        json.prettyPrintTo(Serial);

        configFile.close();
      }
    }
  }
  else
  {
    Serial.println(F("failed to mount FS"));
    return false;
  }
  return true;
}

bool saveSPIFFSConfigFile(void)
{
  Serial.println(F("Saving config"));

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"]   = mqtt_port;

  File configFile = SPIFFS.open(configFileName, "w");

  if (!configFile)
  {
    Serial.println(F("Failed to open config file for writing"));
  }

  //json.printTo(Serial);
  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(configFile);

  configFile.close();
  //end save
}


void setup()
{
  Wire.begin();
  Serial.begin(115200);
  pinMode(DHTPIN, INPUT);
  dht.begin();
  /*
   * LCD
   */
  lcd.init();
  lcd.backlight();

  Serial.println(F("////  DATN  \\\\"));
  
  lcd.setCursor(0,0);
  lcd.print("DATN");
  lcd.setCursor(0,1);
  lcd.print("Starting.....");
  
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pill reminder");
  lcd.setCursor(0,1);
  lcd.print("Version: 1.0");
  
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Everything is");
  lcd.setCursor(0,1);
  lcd.print("almost done");

  delay(500);
  lcd.noBacklight();
  /*
   * Cài đặt MODE cho các pin:
   */
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  //b. nút nhấn:
  pinMode(B_SET, INPUT);
  pinMode(B_PLUS, INPUT);
  pinMode(B_MINUS, INPUT);
  pinMode(B_ONOFF, INPUT);
//  attachInterrupt(digitalPinToInterrupt(B_ONOFF), Bonoff, FALLING);

  //FS
  loadSPIFFSConfigFile();

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  ESP_WMParameter custom_mqtt_server("mqtt_server", "mqtt_server", mqtt_server, MQTT_SERVER_MAX_LEN + 1);
  ESP_WMParameter custom_mqtt_port  ("mqtt_port",   "mqtt_port",   mqtt_port,   MQTT_SERVER_PORT_LEN + 1);

  // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESP_WiFiManager ESP_wifiManager;
  // Use this to personalize DHCP hostname (RFC952 conformed)
  ESP_WiFiManager ESP_wifiManager("AutoConnect-FSParams");

  //set config save notify callback
  ESP_wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  ESP_wifiManager.addParameter(&custom_mqtt_server);
  ESP_wifiManager.addParameter(&custom_mqtt_port);

///////// RESET WIFI /////////////
  //reset settings - for testing
//  ESP_wifiManager.resetSettings(); //just no command this line
///////// RESET WIFI /////////////

  ESP_wifiManager.setDebugOutput(true);

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
  Serial.println("\nStored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);
  
  if (Router_SSID != "")
  {
    ESP_wifiManager.setConfigPortalTimeout(30); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Got stored Credentials. Timeout 30s"));
  }
  else
  {
    Serial.println(F("No stored Credentials. No timeout"));
  }

  String chipID = String(ESP_getChipId(), HEX);
  chipID.toUpperCase();

  // SSID and PW for Config Portal
  AP_SSID = "ESP_" + chipID + "_AutoConnectAP";
//  AP_PASS = "MyESP_" + chipID;

  // Get Router SSID and PASS from EEPROM, then open Config portal AP named "ESP_XXXXXX_AutoConnectAP" and PW "MyESP_XXXXXX"
  // 1) If got stored Credentials, Config portal timeout is 60s
  // 2) If no stored Credentials, stay in Config portal until get WiFi Credentials
  if (!ESP_wifiManager.autoConnect(AP_SSID.c_str(), NULL))
  {
    Serial.println(F("failed to connect and hit timeout"));
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println(F("WiFi connected ... yeey"));

  //read updated parameters
  strncpy(mqtt_server, custom_mqtt_server.getValue(), sizeof(mqtt_server));
  strncpy(mqtt_port, custom_mqtt_port.getValue(),     sizeof(mqtt_port));

  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    saveSPIFFSConfigFile();
  }

  Serial.println(F("\nlocal ip"));
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
  
//  delay(1000);
  connectmqtt();
  //
  menu = 0;

  lcd.backlight();
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Done!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);

  lcd.createChar(1, temperature);
  lcd.createChar(2, humidity);
}



void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
///////////// button SET : menu /////////////////
  int readingSET = digitalRead(B_SET);
  if(readingSET != lastButtonStateSET)
  {
    lastDebounceTimeSET = millis();
  }
  if((millis() - lastDebounceTimeSET) > debounceDelaySET)
  {
    if(readingSET != buttonStateSET)
    {
      buttonStateSET = readingSET;
      if(buttonStateSET == LOW)
      {
        menu++;
        Serial.print("menu (B_SET pressed): ");
        Serial.println(menu);
      }
    }
  }
  lastButtonStateSET = readingSET;
  
///////////// menu = 0 /////////////////
  if(menu == 0)
  {
    DisplayDHT();
    printAlarmLEFT();
    if(REMINDS == 1)
    {
      menu = 4;
    }
  }
  
//////////////// menu = 1: set HOUR ////////////////
  if(menu == 1)
  {
//////////// button B_PLUS ////////////////
    int readingPL = digitalRead(B_PLUS);
    if(readingPL != lastButtonStatePL)
    {
      lastDebounceTimePL = millis();
    }
    if((millis() - lastDebounceTimePL) > debounceDelayPL)
    {
      if(readingPL != buttonStatePL)
      {
        buttonStatePL = readingPL;
        if(buttonStatePL == LOW)
        {
          if(alarm_hour == 23)
          {
            alarm_hour = 0;
          }
          else
          {
            alarm_hour++;
          }
        }
      }
    }
    lastButtonStatePL = readingPL;
//////////// button B_MINUS ////////////////
    int readingMN = digitalRead(B_MINUS);
    if(readingMN != lastButtonStateMN)
    {
      lastDebounceTimeMN = millis();
    }
    if((millis() - lastDebounceTimeMN) > debounceDelayMN)
    {
      if(readingMN != buttonStateMN)
      {
        buttonStateMN = readingMN;
        if(buttonStateMN == LOW)
        {
          if(alarm_hour == 0)
          {
            alarm_hour = 23;
          }
          else
          {
            alarm_hour--;
          }
        }
      }
    }
    lastButtonStateMN = readingMN;

    if(millis() - lastDisplay >= displayGap)
    {
      //Display set HOUR:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set alarm hour:"); //15char
    
      //Display HOUR:
      lcd.setCursor(0,1);
      if(alarm_hour < 10)
      {
        lcd.write('0');
      }
      lcd.print(alarm_hour, DEC);
      
      lastDisplay = millis();
    }
  }
  
//////////////// menu = 2: set MINUTE ////////////////
  if(menu == 2)
  {
//////////// button B_PLUS ////////////////
    int readingPL = digitalRead(B_PLUS);
    if(readingPL != lastButtonStatePL)
    {
      lastDebounceTimePL = millis();
    }
    if((millis() - lastDebounceTimePL) > debounceDelayPL)
    {
      if(readingPL != buttonStatePL)
      {
        buttonStatePL = readingPL;
        if(buttonStatePL == LOW)
        {
          if(alarm_minute == 59)
          {
            alarm_minute = 0;
          }
          else
          {
            alarm_minute++;
          }
        }
      }
    }
    lastButtonStatePL = readingPL;
//////////// button B_MINUS ////////////////
    int readingMN = digitalRead(B_MINUS);
    if(readingMN != lastButtonStateMN)
    {
      lastDebounceTimeMN = millis();
    }
    if((millis() - lastDebounceTimeMN) > debounceDelayMN)
    {
      if(readingMN != buttonStateMN)
      {
        buttonStateMN = readingMN;
        if(buttonStateMN == LOW)
        {
          if(alarm_minute == 0)
          {
            alarm_minute = 59;
          }
          else
          {
            alarm_minute--;
          }
        }
      }
    }
    lastButtonStateMN = readingMN;

    if(millis() - lastDisplay >= displayGap)
    {
      //Display set MINUTE:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set alarm min.:"); //15char
    
      //Display MINUTE:
      lcd.setCursor(0,1);
      if(alarm_minute < 10)
      {
        lcd.write('0');
      }
      lcd.print(alarm_minute, DEC);

      lastDisplay = millis();
    }
  }

  if(menu == 3)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Saved alarm");

    //Chuyển thời gian hẹn giờ đã nhập về giây:
    alarm_sec = (alarm_hour*60 + alarm_minute)*60;
    sec_to_mqtt = "G" + String(alarm_sec);
    
    sec_to_mqtt.toCharArray(bufferG, (sec_to_mqtt.length())+1);
    client.publish(mqtt_pub_topic, bufferG);

    delay(1000);
    lcd.clear();
    menu = 0;
  }
  if(menu == 4)
  {
    Alarm();
  }
  client.loop();
}

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
      Serial.println(REMINDS);
      Serial.println();
    }
    if((char)payload[0] == 'C')
    {
      pload.remove(0,1);
      String ALeft = pload;
      alarm_left = ALeft.toInt();

      Serial.print("payload[0] = 'C' -> ALeft = ");
      Serial.println(ALeft);
      Serial.print("alarm_left = ");
      Serial.println(alarm_left);
      Serial.println();
    }
    if(((char)payload[0] == 'f' || (char)payload[0] == 'F') && ((char)payload[0] == 'i' || (char)payload[0] == 'I') && ((char)payload[0] == 'n' || (char)payload[0] == 'N') && ((char)payload[0] == 'd' || (char)payload[0] == 'D'))
    {
      REMINDS = 2;
      
      Serial.print("payload in 'Find' - REMINDS: ");
      Serial.println(REMINDS);
      Serial.println();
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
      Serial.println(F("connected in reconnect() function."));
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

void findBox()
{
  if(REMINDS == 2)
  {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("I'm here!"); //9 chars
    lcd.setCursor(4,0);
    lcd.print("I'm here!"); //9 chars

    if(millis() - lastRing >= buzzerGap)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      lastRing = millis();
    }
    else
    {
      digitalWrite(BUZZER_PIN, LOW);
    }
    
    ///////////// button ONOFF : mode_ALARM /////////////////
    int readingOO = digitalRead(B_ONOFF);
    if(readingOO != lastButtonStateOO)
    {
      lastDebounceTimeOO = millis();
    }
    if((millis() - lastDebounceTimeOO) > debounceDelayOO)
    {
      if(readingOO != buttonStateOO)
      {
        buttonStateOO = readingOO;
        if(buttonStateOO == LOW)
        {
          lcd.clear();
          digitalWrite(BUZZER_PIN, LOW);
          REMINDS = 0;
          menu = 0;
          Serial.print("FIND (B_ONOFF pressed)");
        }
      }
    }
    lastButtonStateOO = readingOO;
  }
}

void printAlarmLEFT()
{
  lcd.setCursor(0,1);
  lcd.print("Alarm left: "); //11char

  lcd.print(alarm_left, DEC);
  if(alarm_left < 10)
  {
    lcd.setCursor(13,1);
    lcd.print("   ");
  }
}

//Bật-tắt báo thức
void Alarm()
{
    if(REMINDS == 1)
    {
      mode_ALARM = mode_ON;
      Serial.println(F("Alarm..."));
      if(millis() - lastDisplay >= displayGap)
      {
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("Time to"); //7
        lcd.setCursor(0,1);
        lcd.print("Take medicines!"); //15 ký tự

        lastDisplay = millis();
      }
      
      if(millis() - lastRing >= (buzzerGap - 350))
      {
        digitalWrite(BUZZER_PIN, HIGH);
        lastRing = millis();
      }
      else
      {
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
    else
    {
      digitalWrite(BUZZER_PIN, LOW);
    }
  
    ///////////// button ONOFF : mode_ALARM /////////////////
    int readingOO = digitalRead(B_ONOFF);
    if(readingOO != lastButtonStateOO)
    {
      lastDebounceTimeOO = millis();
    }
    if((millis() - lastDebounceTimeOO) > debounceDelayOO)
    {
      if(readingOO != buttonStateOO)
      {
        buttonStateOO = readingOO;
        if(buttonStateOO == LOW)
        {
          if(mode_ALARM == mode_ON)
          {
            lcd.clear();
            mode_ALARM = mode_OFF;
            digitalWrite(BUZZER_PIN, LOW);
            REMINDS = 0;
            menu = 0;
            Serial.print("reminds (B_ONOFF pressed): ");
            Serial.println(REMINDS);
          }
        }
      }
    }
    lastButtonStateOO = readingOO;
}


void DisplayDHT()
{
  static int temp, hum;
  if(millis() - lastRead >= readInterval)
  {
    
    temp = dht.readTemperature(); //đọc nhiệt độ (Ngưỡng t: 0 - 55 độ C, sai số +-2 độ C) 
    hum = dht.readHumidity(); //đọc độ ẩm (Ngưỡng h: 20% - 90%, sai số +-5%)
    // Check if any reads failed and exit early (to try again).
    if (isnan(hum) || isnan(temp))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    lastRead = millis();
  }

  if(millis() - lastSend >= sendInterval)
  {
    String T = "T" + String(temp);
    String H = "H" + String(hum);

    Serial.println("\nvalue:");
    Serial.println(sec_to_mqtt);
    Serial.println(T);
    Serial.println(H);

    T.toCharArray(bufferT, (T.length()+1));
    H.toCharArray(bufferH, (H.length()+1));

    Serial.println("\nbuff:");
    Serial.println(bufferG);
    Serial.println(bufferT);
    Serial.println(bufferH);
    
    client.publish(mqtt_pub_topic, bufferT);
    client.publish(mqtt_pub_topic, bufferH);
      
    lastSend = millis();
  }

  if(millis() - lastDisplay >= displayGap)
  {
    //in ra màn hình LCD:
    lcd.setCursor(1,0);
    lcd.write(1);

    lcd.setCursor(3,0);
    lcd.print(temp, DEC);
  
    lcd.setCursor(5,0);
    lcd.print((char)223); //degree sign
    lcd.print("C");
  
    lcd.setCursor(9,0);
    lcd.write(2);
  
    lcd.setCursor(11,0);
    lcd.print(hum, DEC);
    lcd.setCursor(13,0);
    lcd.print("%");

    lastDisplay = millis();
  }
}
