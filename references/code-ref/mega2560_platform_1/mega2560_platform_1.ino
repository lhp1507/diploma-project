#include <SerialCommand.h>

//Ổn

// thêm thư viện
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <DHT.h>


/*
 * cài đặt giao tiếp tx rx
 * giữa mega2560 và esp8266
 */
//const byte RX = 0; //RX0 trên Arduino mega2560
//const byte TX = 1; //TX0 trên Arduino mega2560
//SoftwareSerial espSerial(RX, TX); //khai báo tên espSerial sử dụng chân TX RX
//SerialCommand sCmd(espSerial); //khai báo biến sử dụng thư viện Serial Command
SerialCommand sCmd;

/*
 * cài đặt LCD
 */
LiquidCrystal_I2C lcd(0x27,20,4); //khai báo địa chỉ và kích thước lcd 20x4

/*
 * cài đặt cảm biến nhiệt độ, độ ẩm
 * DHT11
 */
const int DHTTYPE = DHT11;  //loại dht sử dụng
const int DHTPIN = 8;       //chân nhận dữ liệu
DHT dht(DHTPIN, DHTTYPE);   //cài đặt sử dụng DHT
int h, t;
/*
 * cài đặt RTC DS1307
 */
RTC_DS1307 rtc;

/*
 * khai báo các chân sử dụng GPIO
 */
//a. Nút nhấn:
int B_set = 22;  //chân 19 [INT.2] là nút nhấn set menu
int B_plus = 23; //chân 2 [INT.4] là nút CỘNG
int B_minus = 24; //chân 3 [INT.5] là nút TRỪ
int B_onoff = 25; //chân 18 [INT.3] là nút bật/tắt báo thức
//b. mạch giải mã chọn hộp thuốc:
const int BUZZER = 11; //Chuông báo thức
const int inputC = 5; //74hc138 - C
const int inputB = 6; //74hc138 - B
const int inputA = 7; //74hc138 - A

/*
 * khai báo các biến sử dụng
 */
//a. khai báo menu là biến volatile vì có thay đổi ở CT ngắt
  /*  menu:
  * 0-chạy bth (Mặc định)
  * 1-cài đặt giờ
  * 2-cài đặt phút
  * 3-lưu báo thức
  */
  volatile int menu = 0;
//b. ngày giờ:
  char daysOfTheWeek[7][7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  String wday; //biến lưu thứ trong tuần
  int hourupg;
  int minupg;
  int yearupg;
  int monthupg;
  int dayupg;
//c. báo thức:
  volatile unsigned long alarm_hour = 0, alarm_minute = 0; //Giữ giờ báo thức mặc định hiện tại là 00:00
  volatile unsigned long alarm_sec_trans = 0; //biến chuyển giờ và phút báo thức thành giây để gửi cho esp8266
  unsigned int mode_ON = 1; //trạng thái báo thức
  unsigned int mode_OFF = 0;
  unsigned int mode_ALARM = 0;
  String REMINDS; //NHẬN CHUỖI "Alarm" từ server
//d. khác:
  unsigned long CurrentTime = 0;
  unsigned long LastClick = 0;
  unsigned long LastDisplay = 0;
  unsigned long LastRead = 0;
  unsigned long LastSend = 0;
  unsigned long Waiting = 10000;  //Dùng để sau 10s thì tắt đọ sáng màn hình lcd
  unsigned long ReadInterval = 1000;  //1 chu kỳ đọc
  unsigned long SendInterval = 5000;  //1 chu kỳ gửi
  unsigned long DisplayGap = 500;
  byte timeOut = 60; //Timeout cho Serial
//e. chuỗi json gửi đi esp:
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer1(capacity); //tạo Buffer json có khả năng chứa tối đa 200 ký tự
  JsonObject& root1  = jsonBuffer1.createObject(); //tạo một biến root mang kiểu json



void setup()
{
  /*
   * mở cổng giao tiếp serial:
   */
  Wire.begin();
  Serial.begin(9600); //khởi tạo Serial để debug
//  Serial.begin(115200); //Serial TX3 RX3
//  Serial.setTimeout(timeOut);
//  espSerial.begin(9600); //khởi tạo espSerial để giao tiếp với esp
  rtc.begin();
  dht.begin();
  /*
   * LCD
   */
  lcd.init();
  lcd.backlight();
  lcd.clear();
  /*
   * In thông báo:
   */
  Serial.println("ready!");
  lcd.setCursor(1,0);
  lcd.print("DATN Dau Quang Phu");
  lcd.setCursor(0,1);
  lcd.print("Starting.....");
  lcd.setCursor(0,2);
  lcd.print("Pill reminder");
  lcd.setCursor(0,3);
  lcd.print("Version: 1.0");
  delay(2000);
  //
  lcd.clear();
  lcd.setCursor(0,0);
  /*
   * Cài đặt MODE cho các pin:
   */
  //a. mạch giải mã 74hc138:
  pinMode(inputC, OUTPUT);
  pinMode(inputB, OUTPUT);
  pinMode(inputA, OUTPUT);
    //đặt trạng thái A,B,C là HIGH để không sáng đèn nào
    //không trùng thứ nào trong tuần
    digitalWrite(inputC, HIGH);
    digitalWrite(inputB, HIGH);
    digitalWrite(inputA, HIGH);
  //b. nút nhấn:
  pinMode(B_set, INPUT_PULLUP);
  pinMode(B_plus, INPUT_PULLUP);
  pinMode(B_minus, INPUT_PULLUP);
  pinMode(B_onoff, INPUT_PULLUP);
  /*
   * Một số hàm trong thư viện Serial Command:
   */
  sCmd.addCommand("RM", reminds);
  //
  if(!rtc.begin())
  {
    Serial.println("Couldn't find RTC!");
  }
  //
  if(!rtc.isrunning())
  {
    Serial.println("RTC is not running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop()
{
  CurrentTime = millis();
  
  if(digitalRead(B_set)== LOW) 
  {
    while(digitalRead(B_set)== LOW);
    menu = menu + 1;
  }
  if((digitalRead(B_plus)== LOW)&&(digitalRead(B_minus)== LOW))
  {
    while((digitalRead(B_plus)== LOW)&&(digitalRead(B_minus)== LOW));
    lcd.clear();
    DisplaySetHourAlarm();
    DisplaySetMinuteAlarm();
    //lưu báo thức để gửi cho esp
    //Chuyển thời gian hẹn giờ đã nhập về giây:
    alarm_sec_trans = (alarm_hour*24 + alarm_minute)*60;
    //Lưu giây vào chuỗi json root -> ví dụ {"G":15060}
    root1["G"] = alarm_sec_trans;
    
    lcd.clear();
    lcd.setCursor(7,0);
    lcd.print("ALARM");
    lcd.setCursor(7,1);
    if(alarm_hour <= 9)
    {
    lcd.print("0");
    }
    lcd.print(alarm_hour, DEC);
    lcd.print(":");
    if(alarm_minute <= 9)
    {
    lcd.print("0");
    }
    lcd.print(alarm_minute, DEC);
    delay(1000);
    lcd.clear();
  }
  //menu = 0, chạy bình thường:
  if(menu == 0)
  {
      DisplayDateTime();
      DisplayDHT();
      Aonoff();
      sCmd.readSerial();
    //* gửi dữ liệu 5s 1 lần
    if(CurrentTime - LastSend >= SendInterval)
    {
//      String messageG = "G "+String(alarm_sec_trans);
//      Serial.print("\nmessageG: ");
//      Serial.println(messageG);
//
//      String messageT = "T "+String(t);
//      Serial.print("\nmessageT: ");
//      Serial.println(messageT);
//
//      String messageH = "H "+String(h);
//      Serial.print("\nmessageH: ");
//      Serial.println(messageH);

      Serial.print("TP"); //gửi tên lệnh
      Serial.print('\r'); //gửi \r
      root1.printTo(Serial); //gửi chuỗi Json cho esp
      Serial.print('\r'); //gửi \r
      Serial.println("\nđã gửi root1!");
      
      LastSend = millis();
    }
  }
  if(menu == 1)
  {
    lcd.clear();
    DisplaySetHour();
  }
  if(menu == 2)
  {
    lcd.clear();
    DisplaySetMinute();
  }
  if(menu == 3)
  {
    lcd.clear();
    DisplaySetDay();
  }
  if(menu == 4)
  {
    lcd.clear();
    DisplaySetMonth();
  }
  if(menu == 5)
  {
    lcd.clear();
    DisplaySetYear();
  }
  if(menu == 6)
  {
    lcd.clear();
    StoreAgg();
    menu = 0;
  }
}

void DisplayDateTime()
{
  DateTime now = rtc.now();
  //
  lcd.setCursor(0, 0);
  lcd.print(now.day(), DEC);
  dayupg = now.day();
  lcd.print("/");
  lcd.print(now.month(), DEC);
  monthupg=now.month();
  lcd.print("/");
  lcd.print(now.year(), DEC);
  yearupg=now.year();
  lcd.print(" (");
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  wday = String(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.println(")");
  //
  lcd.setCursor(0, 1);
  if(now.hour() <= 9)
  {
    lcd.print("0");
  }
  lcd.print(now.hour(), DEC);
  hourupg = now.hour();
  lcd.print(":");
  if(now.minute() <= 9)
  {
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);
  minupg = now.minute();
  lcd.print(":");
  if(now.second() <= 9)
  {
    lcd.print("0");
  }
  lcd.print(now.second(), DEC);
}

void DisplayDHT()
{
  h = dht.readHumidity(); //đọc độ ẩm (Ngưỡng h: 20% - 90%, sai số +-5%)
  t = dht.readTemperature(); //đọc nhiệt độ (Ngưỡng t: 0 - 55 độ C, sai số +-2 độ C) 
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  //Lưu vào root để gửi đi:
  root1["T"] = t;
  root1["H"] = h;
  //in ra màn hình LCD:
  lcd.setCursor(0,2);
  lcd.print("Nhiet do:");
  lcd.print(t, DEC);
  lcd.setCursor(11,2);
  lcd.print("oC");
  lcd.setCursor(0,3);
  lcd.print("Do am:");
  lcd.setCursor(9,3);
  lcd.print(h, DEC);
  lcd.setCursor(11,3);
  lcd.print("%");
}

void DisplaySetHourAlarm()// Setting the alarm minutes
{
  while(digitalRead(B_set) == HIGH)
  {
//    lcd.clear();
    if(digitalRead(B_plus) == LOW)
    {
      
      if(alarm_hour == 23)
      {
        alarm_hour = 0;
      }
      else
      {
        alarm_hour = alarm_hour + 1;
      }
      while(digitalRead(B_plus) == LOW);
    }
    if(digitalRead(B_minus) == LOW)
    {
      if(alarm_hour == 0)
      {
        alarm_hour = 23;
      }
      else
      {
        alarm_hour = alarm_hour - 1;
      }
      while(digitalRead(B_minus) == LOW);
    }
    lcd.setCursor(0,0);
    lcd.print("Set HOUR alarm:");
    lcd.setCursor(7,1);
    if(alarm_hour <= 9)
    {
      lcd.print("0");
    }
    lcd.print(alarm_hour,DEC);

    lcd.setCursor(9,1);
    lcd.print(":");
    
    lcd.setCursor(10,1);
    if(alarm_minute <= 9)
    {
      lcd.print("0");
    }
    lcd.print(alarm_minute,DEC);
    delay(100);
  }
  delay(100);
}

void DisplaySetMinuteAlarm()// Setting the alarm minutes
{
  while(digitalRead(B_set) == HIGH)
  {
//    lcd.clear();
    if(digitalRead(B_plus) == LOW)
    {
      
      if (alarm_minute == 59)
      {
        alarm_minute = 0;
      }
      else
      {
        alarm_minute = alarm_minute + 1;
      }
      while(digitalRead(B_plus) == LOW);
    }
    if(digitalRead(B_minus) == LOW)
    {
      
      if (alarm_minute == 0)
      {
        alarm_minute = 59;
      }
      else
      {
        alarm_minute = alarm_minute - 1;
      }
      while(digitalRead(B_minus) == LOW);
    }
    lcd.setCursor(0,0);
    lcd.print("Set MINUTE alarm:");

    lcd.setCursor(10,1);
    if(alarm_minute <= 9)
    {
      lcd.print("0");
    }
    lcd.print(alarm_minute,DEC);
    delay(100);
  }
  delay(100);
}

//Bật/tắt báo thức
void Aonoff()
{
  if(digitalRead(B_onoff) == LOW)
  {
    while(digitalRead(B_onoff) == LOW);
    if(mode_ALARM == mode_ON)
    {
      mode_ALARM = mode_OFF;
    }
    else if(mode_ALARM == mode_OFF)
    {
      mode_ALARM = mode_ON;
    }
  }
  if(mode_ALARM == mode_OFF)
  {
    lcd.setCursor(15,2);
    lcd.print("Alarm");
    lcd.setCursor(15,3);
    lcd.print(" OFF ");
    
    //tắt đèn
    digitalWrite(inputC, HIGH);
    digitalWrite(inputB, HIGH);
    digitalWrite(inputA, HIGH);
    //tắt chuông
    noTone(BUZZER);
  }
  if(mode_ALARM == mode_ON)
  {
    lcd.setCursor(15,2);
    lcd.print("Alarm");
    lcd.setCursor(15,3);
    if(alarm_hour <= 9)
    {
      lcd.print("0");
    }
    lcd.print(alarm_hour,DEC);
    lcd.print(":");
    if(alarm_minute <= 9)
    {
      lcd.print("0");
    }
    lcd.print(alarm_minute,DEC);

    if(REMINDS == "Alarm")
    {
      Serial.println("Alarm.....");
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("DEN GIO"); //7
      lcd.setCursor(1,1);
      lcd.print("UONG THUOC ROI !!!"); //18 ký tự
      lcd.noBacklight();

      if(wday == "Sun")
      {
        digitalWrite(inputC, HIGH);
        digitalWrite(inputB, HIGH);
        digitalWrite(inputA, LOW);
      }
      if(wday == "Mon")
      {
        digitalWrite(inputC, LOW);
        digitalWrite(inputB, LOW);
        digitalWrite(inputA, LOW);
      }
      if(wday == "Tue")
      {
        digitalWrite(inputC, LOW);
        digitalWrite(inputB, LOW);
        digitalWrite(inputA, HIGH);
      }
      if(wday == "Wed")
      {
        digitalWrite(inputC, LOW);
        digitalWrite(inputB, HIGH);
        digitalWrite(inputA, LOW);
      }
      if(wday == "Thu")
      {
        digitalWrite(inputC, LOW);
        digitalWrite(inputB, HIGH);
        digitalWrite(inputA, HIGH);
      }
      if(wday == "Fri")
      {
        digitalWrite(inputC, HIGH);
        digitalWrite(inputB, LOW);
        digitalWrite(inputA, LOW);
      }
      if(wday == "Sat")
      {
        digitalWrite(inputC, HIGH);
        digitalWrite(inputB, LOW);
        digitalWrite(inputA, HIGH);
      }
      
      tone(BUZZER,880); //play the note "A5" (LA5)
      delay(300);
      tone(BUZZER,698); //play the note "F6" (FA5)
      lcd.backlight();
    }
    else
    {
      noTone(BUZZER);
      digitalWrite(inputC, HIGH);
      digitalWrite(inputB, HIGH);
      digitalWrite(inputA, HIGH);
    }
  }
}

void DisplaySetHour()
{
// time setting
//  lcd.clear();
  DateTime now = rtc.now();
  if(digitalRead(B_plus)==LOW)
  {
    
    if(hourupg == 23)
    {
      hourupg = 0;
    }
    else
    {
      hourupg = hourupg + 1;
    }
    while(digitalRead(B_plus)==LOW);
  }
  if(digitalRead(B_minus) == LOW)
  {
    
    if(hourupg == 0)
    {
      hourupg = 23;
    }
    else
    {
      hourupg = hourupg - 1;
    }
    while(digitalRead(B_minus)==LOW);
  }
  lcd.setCursor(0,0);
  lcd.print("Set hour:");
  lcd.setCursor(0,1);
  if(hourupg <= 9)
  {
    lcd.print("0");
  }
  lcd.print(hourupg,DEC);
  delay(100);
}

void DisplaySetMinute()
{
// Setting the minutes
//  lcd.clear();
  if(digitalRead(B_plus)==LOW)
  {
    
    if (minupg==59)
    {
      minupg=0;
    }
    else
    {
      minupg=minupg+1;
    }
    while(digitalRead(B_plus)==LOW);
  }
  if(digitalRead(B_minus)==LOW)
  {
    
    if (minupg==0)
    {
      minupg=59;
    }
    else
    {
      minupg=minupg-1;
    }
    while(digitalRead(B_minus)==LOW);
  }
  lcd.setCursor(0,0);
  lcd.print("Set minutes:");
  lcd.setCursor(0,1);
  if(minupg <= 9)
  {
    lcd.print("0");
  }
  lcd.print(minupg,DEC);
  delay(100);
}

void DisplaySetDay()
{
// Setting the day
//  lcd.clear();
  if(digitalRead(B_plus)==LOW)
  {
    
    if (dayupg==31)
    {
      dayupg=1;
    }
    else
    {
      dayupg=dayupg+1;
    }
    while(digitalRead(B_plus)==LOW);
  }
  if(digitalRead(B_minus)==LOW)
  {
    
    if (dayupg==1)
    {
      dayupg=31;
    }
    else
    {
      dayupg=dayupg-1;
    }
    while(digitalRead(B_minus)==LOW);
  }
  lcd.setCursor(0,0);
  lcd.print("Set Day:");
  lcd.setCursor(0,1);
  lcd.print(dayupg,DEC);
  delay(100);
}

void DisplaySetMonth()
{
// Setting the month
//  lcd.clear();
  if(digitalRead(B_plus)==LOW)
  {
    
    if (monthupg==12)
    {
      monthupg=1;
    }
    else
    {
      monthupg=monthupg+1;
    }
    while(digitalRead(B_plus)==LOW);
  }
  if(digitalRead(B_minus)==LOW)
  {
    
    if (monthupg==1)
    {
      monthupg=12;
    }
    else
    {
      monthupg=monthupg-1;
    }
    while(digitalRead(B_minus)==LOW);
  }
  lcd.setCursor(0,0);
  lcd.print("Set Month:");
  lcd.setCursor(0,1);
  lcd.print(monthupg,DEC);
  delay(100);
}

void DisplaySetYear()
{
// setting the year
//  lcd.clear();
  if(digitalRead(B_plus)==LOW)
  {
    
    yearupg=yearupg+1;
    while(digitalRead(B_plus)==LOW);
  }
   if(digitalRead(B_minus)==LOW)
  {
    yearupg=yearupg-1;
    while(digitalRead(B_minus)==LOW);
  }
  lcd.setCursor(0,0);
  lcd.print("Set Year:");
  lcd.setCursor(0,1);
  lcd.print(yearupg,DEC);
  delay(100);
}

void StoreAgg()
{
// Variable saving
//  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SAVING IN");
  lcd.setCursor(0,1);
  lcd.print("PROGRESS");
  rtc.adjust(DateTime(yearupg,monthupg,dayupg,hourupg,minupg,0));
  delay(200);
}

//--------------------------------------------------//

/*
 * nhận dữ liệu từ esp8266 qua TX RX
 */
void reminds()
{
  Serial.println("RM");
    const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
    DynamicJsonBuffer jsonBuffer2(capacity); //tạo Buffer json có khả năng chứa tối đa 200 ký tự
    
    char *json2 = sCmd.next(); //Chỉ cần 1 dòng này để đọc tham số nhận được
    JsonObject& root2  = jsonBuffer2.parseObject(json2); //tạo một biến root mang kiểu json

    if(!root2.success())
    {
      Serial.println("JSON parsing failed");
    }
    
    const char* reminds = root2["reminds"]; //Alarm
    REMINDS = String(reminds); //lưu vào biến chính
    Serial.print("REMINDS: ");
    Serial.println(REMINDS);
}
//void reminds()
//{
//  Serial.println("REMINDS");
//  char *arg;
//  arg = sCmd.next();
//  if(arg != NULL)
//  {
//    REMINDS = atoi(arg);
//    Serial.print("Reminds esp gửi nè: ");
//    Serial.println(REMINDS);
//  }
//  else
//  {
//    Serial.println("arg = NULL");
//  }
//}
