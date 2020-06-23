/*
 * NOTE:
 * See pin OK to use: https://espeasy.readthedocs.io/en/latest/Reference/GPIO.html#best-pins-to-use-on-esp32
 * GPIO12 -> OK OUTPUT (Boot fail if pulled HIGH)
 * Tụ 10uF gắn vào EN để auto boot
 * Pin map:
 *    Device  <----->  ESP32  --->  Purpose
 *    DHT11   <----->   19    --->  INPUT
 *    BUZZER  <----->   25    --->  OUTPUT
 *    B_SET   <----->   36    --->  INPUT
 *    B_PLUS  <----->   39    --->  INPUT
 *    B_MINUS <----->   34    --->  INPUT
 *    B_OFF   <----->   35    --->  INPUT
 *    SDA     <----->   21    --->  OUTPUT
 *    SCL     <----->   22    --->  OUTPUT
 */
 
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>        //https://github.com/bblanchon/ArduinoJson
#include <ESP_WiFiManager.h>    //https://github.com/khoih-prog/ESP_WiFiManager

#include <WiFi.h>

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <DHT.h>
/*
 * Cấu hình DHT11 
 */
#define DHTTYPE   DHT11     //loại dht sử dụng
#define DHTPIN    19        //chân nhận dữ liệu
DHT dht(DHTPIN, DHTTYPE);   //cài đặt sử dụng DHT
/*
 * Kết thúc cấu hình DHT11
 */

/*
 * Định nghĩa các chân tín hiệu
 */
#define BUZZER_PIN      25
#define BUZZER_CHANNEL  0
#define B_SET     36    //nút set menu
#define B_PLUS    39    //nút TĂNG
#define B_MINUS   34    //nút GIẢM
#define B_OFF     35    //nút TẮT báo thức
#define B_BOX     32    //nút xử lý hộp thuốc
/*
 * Kết thúc định nghĩa chân tín hiệu
 */

//----- mode nhắc nhở (Bật/Tắt) -----//
#define mode_ON   1
#define mode_OFF  0

//Lấy thông tin ID của Chip trong ESP32
#define ESP_getChipId() ((uint32_t)ESP.getEfuseMac())

//----- Định nghĩa độ dài tối đa của Tên Server và Port MQTT -----//
#define MQTT_SERVER_MAX_LEN   40
#define MQTT_SERVER_PORT_LEN  6

//----- Các Topic subscribe và publish của MQTT -----//
#define mqtt_sub_reminds  "Reminds"
#define mqtt_pub_topic    "Topic"

/*
 * Khởi tạo các biểu tượng cho LCD:
 */
//----- Biểu tượng nhiệt độ -----//
byte temperature[8] = { B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110 };
//----- Biểu tượng độ ẩm -----//
byte humidity[8]    = { B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110 };
/*
 * Kết thúc khởi tạo các biểu tượng cho LCD
 */
 
/*
 * Cấu hình các nút nhấn
 */
//----- nút SET - menu điều khiển -----//
int buttonStateSET;                     // the current reading from the input pin
int lastButtonStateSET = LOW;           // the previous reading from the input pin
unsigned long lastDebounceTimeSET = 0;  // the last time the output pin was toggled
unsigned long debounceDelaySET = 50;    // the debounce time; increase if the output flickers

//----- nút PLUS - tăng -----//
int buttonStatePL;                      // the current reading from the input pin
int lastButtonStatePL = LOW;            // the previous reading from the input pin
unsigned long lastDebounceTimePL = 0;   // the last time the output pin was toggled
unsigned long debounceDelayPL = 50;     // the debounce time; increase if the output flickers

//----- nút MINUS - giảm -----//
int buttonStateMN;                      // the current reading from the input pin
int lastButtonStateMN = LOW;            // the previous reading from the input pin
unsigned long lastDebounceTimeMN = 0;   // the last time the output pin was toggled
unsigned long debounceDelayMN = 50;     // the debounce time; increase if the output flickers

//----- nút OFF - tắt báo thức -----//
int buttonStateOO;                      // the current reading from the input pin
int lastButtonStateOO = LOW;            // the previous reading from the input pin
unsigned long lastDebounceTimeOO = 0;   // the last time the output pin was toggled
unsigned long debounceDelayOO = 50;     // the debounce time; increase if the output flickers

//----- nút BOX - nhận biết hộp đựng thuốc có được gắn vào hay không -----//
int buttonStateBOX;                     // the current reading from the input pin
int lastButtonStateBOX = LOW;           // the previous reading from the input pin
unsigned long lastDebounceTimeBOX = 0;  // the last time the output pin was toggled
unsigned long debounceDelayBOX = 50;    // the debounce time; increase if the output flickers
bool BOX = true;                        // true - Box put on ; false - Box put off
/*
 * Kết thúc cấu hình các nút nhấn
 */

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// SSID and PW for Config Portal (Access Point mode)
String AP_SSID;
String AP_PASS;

char mqtt_server [MQTT_SERVER_MAX_LEN];
char mqtt_port   [MQTT_SERVER_PORT_LEN] = "1883";

WiFiClient espClient;
PubSubClient client(espClient);   //lib required for mqtt

//----- cấu hình LCD - SDA: GPIO21; SCL: GPIO22 -----//
LiquidCrystal_I2C lcd(0x27,16,2); //khai báo địa chỉ và kích thước lcd 16x2

/*
 * Khai báo các biến sử dụng
 */
//----- Biến menu có nhiệm vụ thay đổi các nội dung cần thiết lập -----//
  static volatile uint8_t menu;

//----- các biến sử dụng cho thời gian nhắc nhở -----//
  static volatile unsigned long remind_hour = 0, remind_minute = 0; //Giữ giờ nhắc nhở mặc định hiện tại là 00:00
  static volatile unsigned long remind_sec;                         //Lưu giá trị thời gian nhắc nhở dưới dạng giây
  String sec_to_mqtt;                                               //Gửi giá trị của remind_sec lên Broker
  unsigned int remind_left[2] = {0,0};                              //Số nhắc nhở còn lại
  bool remind_left_bool = false;
  static volatile byte mode_REMIND = mode_ON;
  /*
   * REMINDS:
   * 0 - không có gì
   * 1 - chuỗi "Alarm" nhận từ Broker
   * 2 - tìm hộp thuốc
   */
  static volatile uint8_t REMINDS = 0;
  
//----- Các biến khác -----//
  unsigned long lastSend = 0;
  unsigned long lastRead = 0;
  unsigned long lastDisplay = 0;
  unsigned long readInterval = 10000; //1 chu kỳ đọc là 10s
  unsigned long sendInterval = 10000; //1 chu kỳ gửi là 10s
  unsigned long displayGap = 150;     //Chu kỳ hiển thị của LCD
  unsigned long pressTime = 0;        //Thời gian kể từ khi nhấn nút
  unsigned long backlightGap = 10000; //sau 10s không nhấn nút thì tắt đèn nền
  char bufferG[20];                   //Chứa giá trị giây để publish lên broker
  char bufferT[5];                    //Chứa giá trị nhiệt độ để publish lên broker
  char bufferH[5];                    //Chứa giá trị độ ẩm để publish lên broker
/*
 * Kết thúc khai báo các biến sử dụng
 */
 
char configFileName[] = "/config.json"; //name for config file
bool shouldSaveConfig = false;          //flag for saving data

//---------- Hàm set biến lưu dữ liệu config ----------//
void saveConfigCallback(void)
{
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

//---------- Hàm đọc dữ liệu từ file Config ----------//
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

//---------- Hàm lưu dữ liệu mqtt server và port vào file Config ----------//
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

//---------- Hàm setup - Cấu hình và khởi tạo các giá trị ban đầu ----------//
void setup()
{
  //------ Mở các cổng kết nối -----//
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  lcd.init();
  lcd.backlight();

  Serial.println(F("--------- DATN ---------"));
  
  lcd.setCursor(0,0);
  lcd.print("Phu Dau Quang"); //13
  lcd.setCursor(0,1);
  lcd.print("Starting.");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  delay(500);
  lcd.print(".");
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pill reminder");
  lcd.setCursor(0,1);
  lcd.print("Version: 1.0");
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Everything is");
  lcd.setCursor(0,1);
  lcd.print("almost done");
  delay(1000);
  lcd.noBacklight();

  //------ Cấu hình cho các PIN -----//
  pinMode(DHTPIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(B_SET, INPUT);
  pinMode(B_PLUS, INPUT);
  pinMode(B_MINUS, INPUT);
  pinMode(B_OFF, INPUT);
  pinMode(B_BOX, INPUT_PULLUP);

  //FS
  loadSPIFFSConfigFile();

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  ESP_WMParameter custom_mqtt_server("mqtt_server", "mqtt_server", mqtt_server, MQTT_SERVER_MAX_LEN + 1);
  ESP_WMParameter custom_mqtt_port  ("mqtt_port",   "mqtt_port",   mqtt_port,   MQTT_SERVER_PORT_LEN + 1);

  // Use this to personalize DHCP hostname (RFC952 conformed)
  ESP_WiFiManager ESP_wifiManager("AutoConnect-FSParams");

  //set config save notify callback
  ESP_wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  ESP_wifiManager.addParameter(&custom_mqtt_server);
  ESP_wifiManager.addParameter(&custom_mqtt_port);

//----- RESET WIFI - for testing -----//
//  ESP_wifiManager.resetSettings(); //just no command this line
//----- RESET WIFI - for testing -----//

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

  // SSID for Config Portal
  AP_SSID = "ESP_" + chipID + "_AutoConnectAP";

  // Get Router SSID and PASS from EEPROM, then open Config portal AP named "ESP_XXXXXX_AutoConnectAP"
  // 1) If got stored Credentials, Config portal timeout is 30s
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

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WIFI connected"); //14
  delay(1000);
  lcd.clear();
  
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
  
  connectmqtt();

  menu = 0;

  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Done!");

  lcd.createChar(1, temperature);
  lcd.createChar(2, humidity);
  
  delay(1000);
  lcd.clear();
}

//---------- Hàm loop - chương trình chính ----------//
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  //----- button SET : menu -----//
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
        lcd.backlight();
        pressTime = millis();
        
        menu++;
        Serial.print("menu (B_SET pressed): ");
        Serial.println(menu);
      }
    }
  }
  lastButtonStateSET = readingSET;

  
  //----- menu = 0 : Hiển thị nhiệt độ, độ ẩm, số nhắc nhở còn lại, kiểm tra BOX, kiểm tra REMINDS -----//
  if(menu == 0)
  {
    DisplayDHT();
    printRemindLeft();

    //----- button BOX : kiểm tra xem box đựng thuốc có được gắn hay không -----//
    int readingBOX = digitalRead(B_BOX);
    if(readingBOX != lastButtonStateBOX)
    {
      lastDebounceTimeBOX = millis();
    }
    if((millis() - lastDebounceTimeBOX) > debounceDelayBOX)
    {
      if(readingBOX != buttonStateBOX)
      {
        buttonStateBOX = readingBOX;
        if(buttonStateBOX == LOW)
        {
          BOX = true;
          Serial.println("B_BOX pressed: true");
        }
        else
        {
          BOX = false;
          Serial.println("B_BOX unpressed: false");
        }
      }
    }
    lastButtonStateBOX = readingBOX;
    
    if((REMINDS == 1) && (BOX == true))
    {
      menu = 4;
    }
    if(REMINDS == 2)
    {
      menu = 5;
    }
  }
  
  //----- menu = 1 : set HOUR -----//
  if(menu == 1)
  {
    //----- button B_PLUS -----//
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
          lcd.backlight();
          pressTime = millis();
          
          if(remind_hour == 23)
          {
            remind_hour = 0;
          }
          else
          {
            remind_hour++;
          }
        }
      }
    }
    lastButtonStatePL = readingPL;

    //----- button B_MINUS -----//
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
          lcd.backlight();
          pressTime = millis();
          
          if(remind_hour == 0)
          {
            remind_hour = 23;
          }
          else
          {
            remind_hour--;
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
      lcd.print("Set remind hour:"); //15char
    
      //Display HOUR:
      lcd.setCursor(0,1);
      if(remind_hour < 10)
      {
        lcd.write('0');
      }
      lcd.print(remind_hour, DEC);
      
      lastDisplay = millis();
    }
  }

  //----- menu = 2: set MINUTE -----//
  if(menu == 2)
  {
    //----- button B_PLUS -----//
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
          lcd.backlight();
          pressTime = millis();
          
          if(remind_minute == 59)
          {
            remind_minute = 0;
          }
          else
          {
            remind_minute++;
          }
        }
      }
    }
    lastButtonStatePL = readingPL;

    //----- button B_MINUS -----//
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
          lcd.backlight();
          pressTime = millis();
          
          if(remind_minute == 0)
          {
            remind_minute = 59;
          }
          else
          {
            remind_minute--;
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
      lcd.print("Set remind min.:"); //15char
    
      //Display MINUTE:
      lcd.setCursor(0,1);
      if(remind_minute < 10)
      {
        lcd.write('0');
      }
      lcd.print(remind_minute, DEC);

      lastDisplay = millis();
    }
  }

  //----- menu = 3 : Lưu thời gian nhắc nhở, chuyển về giây và gửi lên Broker -----//
  if(menu == 3)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Saved remind");

    //Chuyển thời gian hẹn giờ đã nhập về giây:
    remind_sec = (remind_hour*60 + remind_minute)*60;
    sec_to_mqtt = "G" + String(remind_sec);
    
    sec_to_mqtt.toCharArray(bufferG, (sec_to_mqtt.length() + 1));
    client.publish(mqtt_pub_topic, bufferG);

    delay(1000);
    lcd.clear();
    menu = 0;
  }

  //----- menu = 4 : Nhắc nhở -----//
  if(menu == 4)
  {
    remind();
  }

  //----- menu = 5 : Tìm hộp thuốc -----//
  if(menu == 5)
  {
    findBox();
  }

  //----- 10s sau khi nhấn nút -> tự tắt đèn nền -----//
  if((millis() - pressTime) > backlightGap)
  {
    lcd.noBacklight();
  }
  
  client.loop();
}

//---------- Hàm kết nối vào MQTT ----------//
void connectmqtt()
{
  client.connect("ESP32_PILL_ClientID");  // ESP will connect to mqtt broker with clientID
  {
    Serial.print("connected to MQTT at ");
    Serial.println(mqtt_server);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MQTT connected"); //14
    delay(500);
    lcd.clear();
    
    client.subscribe(mqtt_sub_reminds);
    client.publish(mqtt_pub_topic, "PILLBOX has connected to MQTT");

    if (!client.connected())
    {
      reconnect();
    }
  }
}

//---------- Hàm callback nhận giá trị publisher thông qua broker ----------//
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
    //----- for remind() -----//
    if(pload == "Alarm")
    {
      REMINDS = 1;
      Serial.print("payload in 'Alarm' - REMINDS: ");
      Serial.println(REMINDS);
      Serial.println();
    }

    //----- for printRemindLeft() -----//
    if((char)payload[0] == 'C')
    {
      remind_left[0] = remind_left[1];    //Lưu giá trị remind_left[1] trước đó vào remind_left[0]
      pload.remove(0,1);
      String RLeft = pload;
      remind_left[1] = RLeft.toInt();     //Lưu giá trị remind_left[1] mới
      if((remind_left[1] == 0) && (remind_left[1] != remind_left[0]))
      {
        remind_left_bool = true;
      }
      Serial.print("payload[0] = 'C' -> RLeft = ");
      Serial.println(RLeft);
      Serial.print("remind_left new = ");
      Serial.println(remind_left[1]);
      Serial.print("remind_left old = ");
      Serial.println(remind_left[0]);
      Serial.println();
    }

    //----- for findBox() -----//
    if((char)payload[0] == 'f')
    {
      REMINDS = 2;
      Serial.print("payload in 'find' - REMINDS: ");
      Serial.println(REMINDS);
      Serial.println();
    }
    
    //----- for app -----//
    if((char)payload[0] == 'o')
    {      
      lcd.clear();
      digitalWrite(BUZZER_PIN, LOW);
      REMINDS = 0;
      menu = 0;
      Serial.print("callback off");
    }
  }
}

//---------- Hàm kết nối lại MQTT khi bị mất kết nối ----------//
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (client.connect("ESP32_PILL_ClientID"))
    {
      Serial.println(F("connected in reconnect() function."));
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MQTT reconnected"); //16 char
      delay(500);
      lcd.clear();
      
      // Once connected, publish an announcement...
      client.publish(mqtt_pub_topic, "ESP32_PILL_reconnected");
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

//---------- Hàm hiển thị khi có thông báo tìm hộp thuốc ----------//
void findBox()
{
  if(REMINDS == 2)
  {
    lcd.clear();
    lcd.backlight();
    if(millis() - lastDisplay >= displayGap)
    {
      lcd.setCursor(3,0);
      lcd.print("I'm here!"); //9 chars
      lcd.setCursor(4,1);
      lcd.print("I'm here!"); //9 chars

      lastDisplay = millis();
    }

    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);

    //----- button OFF : mode_REMIND -----//
    int readingOO = digitalRead(B_OFF);
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
          lcd.backlight();
          pressTime = millis();
          
          lcd.clear();
          digitalWrite(BUZZER_PIN, LOW);
          REMINDS = 0;
          menu = 0;
          
          client.publish(mqtt_pub_topic, "off");
          
          Serial.print("FIND (B_OFF pressed)");
        }
      }
    }
    lastButtonStateOO = readingOO;
  }
}

//---------- Hàm nhắc nhở uống thuốc khi đến giờ ----------//
void remind()
{
    if((REMINDS == 1) && (BOX == true))
    {
      mode_REMIND = mode_ON;
      Serial.println(F("Reminding..."));
      lcd.backlight();
      if(millis() - lastDisplay >= displayGap)
      {
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("Time to"); //7
        lcd.setCursor(0,1);
        lcd.print("Take medicines!"); //15 ký tự

        lastDisplay = millis();
      }
      
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      delay(100);
    }
    else
    {
      digitalWrite(BUZZER_PIN, LOW);
    }
  
    //----- button OFF : mode_REMIND -----//
    int readingOO = digitalRead(B_OFF);
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
          if(mode_REMIND == mode_ON)
          {
            lcd.backlight();
            pressTime = millis();
            
            lcd.clear();
            mode_REMIND = mode_OFF;
            digitalWrite(BUZZER_PIN, LOW);
            REMINDS = 0;
            menu = 0;

            client.publish(mqtt_pub_topic, "off");
            
            Serial.print("reminds (B_OFF pressed): ");
            Serial.println(REMINDS);
          }
        }
      }
    }
    lastButtonStateOO = readingOO;
}

//---------- Hàm hiển thị số nhắc nhở còn lại ----------//
void printRemindLeft()
{
  lcd.setCursor(0,1);
  lcd.print("Remind left: "); //13char
  lcd.print(remind_left[1], DEC);
  if(remind_left[1] < 10)
  {
    lcd.setCursor(14,1);
    lcd.print("  ");
  }
  else if(remind_left[1] < 100)
  {
    lcd.setCursor(15,1);
    lcd.print(" ");
  }

  //Nếu số nhắc nhở hiện tại = 0 (hết số nhắc nhở trong ngày)
  //và khác với số nhắc nhở trước đó thì in ra thông báo thêm thuốc vào hộp
  if(remind_left_bool == true)
  {
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Please add");  //10char
    lcd.setCursor(0,1);
    lcd.print("medicines");   //9char

    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);

    //----- button OFF : mode_REMIND -----//
    int readingOO = digitalRead(B_OFF);
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
          lcd.backlight();
          pressTime = millis();
          remind_left_bool = false;
          lcd.clear();
          digitalWrite(BUZZER_PIN, LOW);
          menu = 0;
        }
      }
    }
    lastButtonStateOO = readingOO;
  }
}

//---------- Hàm hiển thị nhiệt độ, độ ẩm và gửi lên Broker qua topic ----------//
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
