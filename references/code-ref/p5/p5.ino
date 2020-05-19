// Đồng hồ báo thức

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <HCMatrixKeypad.h>
#define DEBOUNCE 10

#define C1 2 /* Cột 1 */
#define C2 3 /* Cột 2 */
#define C3 4
#define C4 5
#define C5 6
#define C6 7
#define R1 8 /* Hàng 1 */

#define B_Auto 6
#define B_Menu 5
#define B_Left 4
#define B_Right 3
#define B_Select 2
#define B_On 1

#define Norm 0
#define Menu 1
#define Set_h 2
#define Set_m 3
#define Alarm_fix 4
#define Alarm_tog 5

#define Mode_ON 1
#define Mode_OFF 0
#define Pulse 12

// bàn phím 1x6 keypad
HCMatrixKeypad Keypad(DEBOUNCE, C1, C2, C3, C4, C5, C6, ROWMARKER, R1);
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 rtc;

char daysOfTheWeek[7][7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

uint8_t BRow = 0;
uint8_t BCol = 0;

uint8_t State = 0;
uint8_t A_hour = 7;
uint8_t A_min = 0;
uint8_t Alarm = 0;
uint8_t Buzz = 0;

unsigned long CurrentTime = 0;
unsigned long LastClick = 0;
unsigned long LastDisplay = 0;
unsigned long LastRead = 0;
unsigned long Waiting = 10000;
unsigned long ReadInterval = 1000;
unsigned long DisplayGap = 500;

void setup()
{
  Serial.begin(9600);
 //
  lcd.init();
 //
  lcd.backlight();
 //
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  lcd.setCursor(0, 1);
  lcd.print("Alarm clock 1.0 ");
  delay(1000);
  //
  lcd.clear();
  lcd.setCursor(0, 0);
  //
  if (! rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    
  }
 //
  if (! rtc.isrunning())
  {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
 //
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Pulse, OUTPUT);
}
//
void loop()
{
  Keypad.Scan();
  //
  if (Keypad.New_Key())
  {
    BCol = Keypad.Read() % 10;
    Serial.print(" Col ");
    Serial.println(BCol);
    //
    LastClick = millis();
  }
  //
  CurrentTime = millis();
  // State 0 Bình thường
  if (State == Norm)
  {
  // Hiển thị thời gian
  // Cách khoảng thời gian đọc 1s
    if (CurrentTime - LastRead >= ReadInterval)
    {
      DateTime now = rtc.now();
        //
      lcd.setCursor(0, 0);
      lcd.print(now.day(), DEC);
      lcd.print('/');
      lcd.print(now.month(), DEC);
      lcd.print('/');
      lcd.print(now.year(), DEC);
      lcd.print(" (");
      lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
      lcd.println(") ");
      //
      lcd.setCursor(0, 1);
      lcd.print(now.hour(), DEC);
      lcd.print(':');
      lcd.print(now.minute(), DEC);
      lcd.print(':');
      lcd.print(now.second(), DEC);
      lcd.println(" ");
      //
      LastRead = millis();
      // Kiểm tra chế độ báo thức
      if(Alarm == Mode_ON)
      {
        if((now.hour() == A_hour) && (now.minute() == A_min))
        {
          Serial.println("Alarm...");
          lcd.setCursor(0, 0);
          lcd.print("Alarm... ");
          digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
          //
          if(Buzz == 0)
          {
            Buzz = 1;
            tone(Pulse, 440);
          }
          else
          {
            Buzz = 0;
            tone(Pulse, 550);
          }
        }
      }
      else
      {
        noTone(Pulse);
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    //
    if (BCol == B_Menu)
    {
      State = Menu;
    //
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.println("Alarm menu ");
      lcd.setCursor(0, 1);
      lcd.print("1* ");
      lcd.print(A_hour, DEC);
      lcd.print(" :");
      lcd.print(A_min, DEC);
      lcd.println(" ");
      //
      BCol = 0;
    }
    //
    if (BCol == B_On)
    {
      BCol = 0;
      State = Alarm_tog;
    }
  }
  //
  else if (State == Menu)
  {
    if (CurrentTime - LastClick >= Waiting)
    {
      State = Norm;
    }
  //
  if (BCol == B_Select)
  {
    State = Set_h;
 //
    BCol = 0;
 //
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Setting hour ");
    lcd.setCursor(0, 1);
    lcd.print(A_hour, DEC);
    lcd.print(" :");
    lcd.print(A_min, DEC);
    lcd.println(" ");
  }
 //
  if(BCol == B_Auto)
  {
    State = Norm;
  }
  }
 // Đặt giờ
  else if (State == Set_h)
  {
    if (CurrentTime - LastClick >= Waiting)
    {
      State = Norm;
    }
    //
    if (BCol == B_Right)
    {
      BCol = 0;
    //
      if (A_hour < 24)
      {
        A_hour = A_hour + 1;
      }
      else
      {
        A_hour = 1;
      }
    }
    //
    if (BCol == B_Left)
    {
      BCol = 0;
    //
      if (A_hour > 1)
      {
        A_hour = A_hour - 1;
      }
      else
      {
        A_hour = 24;
      }
    }
    //
    if (BCol == B_Select)
    {
      BCol = 0;
      State = Set_m;
    }
 //
    if (CurrentTime - LastDisplay >= DisplayGap)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.println("Setting hour ");
      lcd.setCursor(0, 1);
      lcd.print(A_hour, DEC);
      lcd.print(" :");
      lcd.print(A_min, DEC);
      lcd.println(" ");
 //
      LastDisplay = millis();
    }
  }
  // Đặt phút
  else if (State == Set_m)
  {
    if (CurrentTime - LastClick >= Waiting)
    {
      State = Norm;
    }
    //
    if (BCol == B_Right)
    {
      BCol = 0;
    //
      if (A_min < 59)
      {
        A_min = A_min + 1;
      }
      else
      {
        A_min = 0;
      }
    }
 //
    if (BCol == B_Left)
    {
      BCol = 0;
 //
      if (A_min > 0)
      {
        A_min = A_min - 1;
      }
      else
      {
        A_min = 59;
      }
    }
 //
    if (BCol == B_Select)
    {
      BCol = 0;
      State = Alarm_fix;
    }
 //
    if (CurrentTime - LastDisplay >= DisplayGap)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.println("Setting minute ");
      lcd.setCursor(0, 1);
      lcd.print(A_hour, DEC);
      lcd.print(" :");
      lcd.print(A_min, DEC);
      lcd.println(" ");
 //
      LastDisplay = millis();
    }
  }
 //
  else if (State == Alarm_fix)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Alarm set ");
    lcd.setCursor(0, 1);
    lcd.print(A_hour, DEC);
    lcd.print(" :");
    lcd.print(A_min, DEC);
    lcd.println(" ");
 //
    Alarm = 1;
    State = Norm;
 //
    delay(1000);
  }
  else if (State == Alarm_tog)
  {
    if (CurrentTime - LastClick >= Waiting)
    {
      State = Norm;
    }
 //
    if(BCol == B_Auto)
    {
      State = Norm;
    }
 //
    if (BCol == B_On)
    {
      BCol = 0;
 //
      if (Alarm == 0)
      {
        Alarm = Mode_ON;
      }
      else
      {
        Alarm = Mode_OFF;
      }
    }
 //
    if (CurrentTime - LastDisplay >= DisplayGap)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.println("Alarm mode ");
      lcd.setCursor(0, 1);
      if(Alarm == Mode_ON)
      {
        lcd.print(" ON ");
      }
      else
      {
        lcd.print(" OFF");
      }
      lcd.println(" ");
 //
      LastDisplay = millis();
    }
  }
}
