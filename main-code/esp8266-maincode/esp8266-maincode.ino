/*
 * D:\KT_MayTinh\Semester 9th DATN\Phu_Dau\Diagrams\ESP-devices.png
 */
 
/*
 * Thanh phan:
 *  dht11
 *  esp8266
 *  lcd2004 - i2c
 *  led - res
 *  74hc138 (demux)
 */
/*
 * Chuc nang:
 *  
 *  
 *  
 *  
 *  
 *  
 */



// ESP8266 WiFi main library
#include <ESP8266WiFi.h>
// Libraries for internet time
#include <WiFiUdp.h>
#include <NTPClient.h>          // include NTPClient library
#include <TimeLib.h>            // include Arduino time library
//74HC138
#define inputC D6
#define inputB D7
#define inputA D8
//Button
#define bt_plus D1   //  Button +         -> interrupt pin
#define bt_minus D2  //  Button -         -> interrupt pin 
#define bt_set D3    //  Button SET ALARM -> interrupt pin

volatile int menu; //khai báo menu là biến volatile vì có thay đổi ở CT ngắt
//tham khảo: https://tapit.vn/y-nghia-cua-tu-khoa-volatile-trong-lap-trinh-nhung-c/
/*  menu:
  * 0-chạy bth (Mặc định)
  * 1-cài hẹn giờ
  * 2-cài đặt giờ
  * 3-cài đặt phút
*/
volatile int ifAlarmed;
/*  Kiểm tra xem đã xảy ra báo thức hay chưa?
  * 0 - chưa xảy ra (Mặc định)
  * 1 - đã xảy ra
*/

// set Wi-Fi SSID and password
const char *ssid     = "Nibotana";
const char *password = "1234567899";

WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server)
//with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
//Other server:
////Vietnam - vn.pool.ntp.org
////server  - "3.vn.pool.ntp.org"
////server  - "1.asia.pool.ntp.org"
////server  - "2.asia.pool.ntp.org"
NTPClient timeClient(ntpUDP, "3.vn.pool.ntp.org", 7*3600, 60000);

//Khai báo các hàm interrupt:
void ICACHE_RAM_ATTR setA(void);
void ICACHE_RAM_ATTR plusA(void);
void ICACHE_RAM_ATTR minusA(void);

char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
volatile byte last_second, second_, minute_, hour_, wday, day_, month_, year_;

uint8_t alarm_hour = 0, alarm_minute = 0;
//Giữ giờ báo thức mặc định hiện tại là 00:00

void setup()
{
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting.");
  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nconnected to -> ");
  Serial.println(ssid);
  Serial.println();
  
  //74HC138
  pinMode(inputC, OUTPUT);
  pinMode(inputB, OUTPUT);
  pinMode(inputA, OUTPUT);
  //đặt trạng thái A,B,C là HIGH để không sáng đèn nào
  //không trùng thứ nào trong tuần
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, HIGH);
  //button
  pinMode(bt_set, INPUT_PULLUP);
  pinMode(bt_plus, INPUT_PULLUP);
  pinMode(bt_minus, INPUT_PULLUP);
  //Interrupt D3 - bt_set; D1 - bt_plus; D2 - bt_minus
  attachInterrupt(digitalPinToInterrupt(bt_set), setA, FALLING);
  attachInterrupt(digitalPinToInterrupt(bt_plus), plusA, FALLING);
  attachInterrupt(digitalPinToInterrupt(bt_minus), minusA, FALLING);
  
  timeClient.begin();

  ifAlarmed = 0;
  menu = 0;
}

void loop()
{
  //Các CT con tương ứng với chỉ số menu 
  if(menu == 0)
  {
    getNTPTime();
    alarm();
  }
  if(menu == 1)
  {
    //Chạy chương trình ngắt plusA()
  }
  if(menu == 2)
  {
    //Chạy chương trình ngắt minusA()
  }
  if(menu >= 3)
  {
    menu = 0;
  }
  delay(1000);
}

//Các hàm con
/*
 * Hàm interrupt
 * Kiểm tra xem nút SET có được nhấn không và tăng chỉ số menu
 */
void ICACHE_RAM_ATTR setA(void)
{
  if(digitalRead(bt_set) == LOW)
  {
    menu++;
    //Chống nhiễu nút nhấn:
    while(digitalRead(bt_set) == LOW);
  }
}
/*
 * Hàm interrupt
 * CỘNG (plus) giờ hoặc phút của báo thức
 */
void ICACHE_RAM_ATTR plusA(void)
{
  if(digitalRead(bt_plus) == LOW)
  {
    if(menu == 1) //Cộng giờ
    {
      if(alarm_hour == 23)
      {
        alarm_hour = 0;
      }
      else
      {
        alarm_hour++;
      }
      Serial.print("hour:");
      Serial.println(alarm_hour);
    }
    if(menu == 2) //Cộng phút
    {
      if(alarm_minute == 59)
      {
        alarm_minute = 0;
      }
      else
      {
        alarm_minute++;
      }
      Serial.print("minute: ");
      Serial.println(alarm_minute);
    }
    //Chống nhiễu nút nhấn:  
    while(digitalRead(bt_plus) == LOW);
  }
}
/*
 * Hàm interrupt
 * TRỪ (minus) giờ hoặc phút của báo thức
 */
void ICACHE_RAM_ATTR minusA(void)
{
  if(digitalRead(bt_minus) == LOW)
  {
    if(menu == 1) //Trừ giờ
    {
      if(alarm_hour == 0)
      {
        alarm_hour = 23;
      }
      else
      {
        alarm_hour--;
      }
      Serial.print("hour:");
      Serial.println(alarm_hour);
    }
    if(menu == 2) //Trừ phút
    {
      if(alarm_minute == 0)
      {
        alarm_minute = 59;
      }
      else
      {
        alarm_minute--;
      }
      Serial.print("minute: ");
      Serial.println(alarm_minute);
    }
    //Chống nhiễu
    while(digitalRead(bt_minus) == LOW);
  }
}
/*
 * Lấy thời gian thực từ NTP server từng giây
 */
void getNTPTime()
{
  if (WiFi.status() == WL_CONNECTED)  // check WiFi connection status
  {
    timeClient.update();
    
    unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time
    second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
    if (last_second != second_)          // update time & date every 1 second
    {
      minute_ = minute(unix_epoch);      // get minutes (0 – 59)
      hour_   = hour(unix_epoch);        // get hours   (0 – 23)
      wday    = weekday(unix_epoch);     // get weekday (1 – 7 with Sunday is day 1)
      day_    = day(unix_epoch);         // get month day (1 – 31, depends on month)
      month_  = month(unix_epoch);       // get month (1 – 12 with Jan is month 1)
      year_   = year(unix_epoch) - 2000; // get year with 4 digits – 2000 results 2 digits year (ex: 2018 –> 18)
      
      Time[7] = second_ % 10 + '0';
      Time[6] = second_ / 10 + '0';
      Time[4] = minute_ % 10 + '0';
      Time[3] = minute_ / 10 + '0';
      Time[1] = hour_   % 10 + '0';
      Time[0] = hour_   / 10 + '0';
      
      Date[9] = year_   % 10 + '0';
      Date[8] = year_   / 10 + '0';
      Date[4] = month_  % 10 + '0';
      Date[3] = month_  / 10 + '0';
      Date[1] = day_    % 10 + '0';
      Date[0] = day_    / 10 + '0';   

      display_DateTime();
      
      last_second = second_;
    }
  }
  else
  {
    Serial.print("\nCan't connect to WiFi\n");
  }
}
/*
 * in ngày trong tuần:
 */
void display_wday()
{
  switch(wday)
  {
    case 1:  Serial.print("SUNDAY    "); break;
    case 2:  Serial.print("MONDAY    "); break;
    case 3:  Serial.print("TUESDAY   "); break;
    case 4:  Serial.print("WEDNESDAY "); break;
    case 5:  Serial.print("THURSDAY  "); break;
    case 6:  Serial.print("FRIDAY    "); break;
    default: Serial.print("SATURDAY  ");
  }
}
/*
 * in ngày giờ đã nhận từ NTP server ra màn hình
 */
void display_DateTime()
{
  Serial.print("Time ");
  Serial.print(Time);
  Serial.print("  --  ");
  display_wday();
  Serial.print(Date);
  Serial.println();
}
/*
 * chọn hộp thuốc theo ngày trong tuần wday
 */
void choose_box()
{
  switch(wday)
  {
    case 1: //Sunday
      digitalWrite(inputC, HIGH);
      digitalWrite(inputB, HIGH);
      digitalWrite(inputA, LOW);
      break;
    case 2: //Monday
      digitalWrite(inputC, LOW);
      digitalWrite(inputB, LOW);
      digitalWrite(inputA, LOW);
      break;
    case 3: //Tuesday
      digitalWrite(inputC, LOW);
      digitalWrite(inputB, LOW);
      digitalWrite(inputA, HIGH);
      break;
    case 4: //Wednesday
      digitalWrite(inputC, LOW);
      digitalWrite(inputB, HIGH);
      digitalWrite(inputA, LOW);
      break;
    case 5: //Thusday
      digitalWrite(inputC, LOW);
      digitalWrite(inputB, HIGH);
      digitalWrite(inputA, HIGH);
      break;
    case 6: //Friday
      digitalWrite(inputC, HIGH);
      digitalWrite(inputB, LOW);
      digitalWrite(inputA, LOW);
      break;
    default: //Saturday
      digitalWrite(inputC, HIGH);
      digitalWrite(inputB, LOW);
      digitalWrite(inputA, HIGH);
  }
}
/*
 * in giờ báo thức đã cài đặt ra màn hình
 */
void display_alarm()
{
  Serial.print("Alarm time -> ");
  /*
   * Hiển thị giờ báo thức theo 00:00
   */
  if(alarm_hour <= 9)
  {
    Serial.print("0");
  }
  Serial.print(alarm_hour);
  
  Serial.print(":");
  
  /*
   * Hiển thị giờ báo thức theo 00:00
   */
  if(alarm_minute <= 9)
  {
    Serial.print("0");
  }
  Serial.print(alarm_minute);
  
  Serial.println("\n");
}
void alarm()
{
  display_alarm();
  /*
   * kiểm tra xem đã xảy ra báo thứ hay chưa
   * nếu chưa (ifAlarmed = 0) thì kiểm tra xem đến giờ báo thức không
   * nếu nếu đã xảy ra báo thức rồi, sau 10s tắt báo thức (chỉ tắt in ra Serial)
   */
  if(ifAlarmed == 0)
  {
    if((alarm_hour == hour_) && (alarm_minute == minute_))
    {
      choose_box();
      Serial.print("\n\n...Giờ ăn tới rồi giờ ăn tới rồi...\n\n");
      Serial.println("done");
      ifAlarmed = 1;
    }
  }
  if(ifAlarmed == 1)
  {
    delay(10000);
    Serial.println("đã xóa done sau 10s");
    //Tắt đèn báo:
    digitalWrite(inputC, HIGH);
    digitalWrite(inputB, HIGH);
    digitalWrite(inputA, HIGH);
    ifAlarmed = 2;
  }
  if(ifAlarmed == 2)
  {
    if((alarm_hour == hour_) && (alarm_minute == minute_))
    {
      Serial.println("đã tat bao thuc");
    }
    else
    {
      ifAlarmed = 0;
    }
  }
}

// End of code.
