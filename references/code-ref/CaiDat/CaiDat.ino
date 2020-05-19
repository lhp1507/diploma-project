#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2); 
uint8_t DegreeBitmap[]= { 0x6, 0x9, 0x9, 0x6, 0x0, 0, 0, 0 };
// từ khối khác 
int M = 0; // biến mode hiển thị ( 0: Choos, 1: dismiss)
int x = 5; // hàng chục ( nhiệt độ )
int y = 0; // hàng đơn vị ( nhiệt độ)
float z = 5; // hàng phần chục ( nhiệt độ)
float Tc = 0; // nhiệt độ cài đặt = 10x + y + z/10

// riêng 
int buttonState0;             // the current reading from the input pin
int lastButtonState0 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime0 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay0 = 50;    // the debounce time; increase if the output flickers

int buttonState1;             // the current reading from the input pin
int lastButtonState1 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay1 = 50;    // the debounce time; increase if the output flickers

int buttonState2;             // the current reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay2 = 50;    // the debounce time; increase if the output flickers
void setup()
{
  Serial.begin(9600);
  lcd.init();                    
  lcd.backlight();
  
  pinMode(11, INPUT); // mode pin 
  pinMode(12, INPUT); // minus pin
  pinMode(13, INPUT); // plus pin
}
void loop() 
{

//////////////////////////////////////////////// button 0 : mode

   int reading0 = digitalRead(0);
  if (reading0 != lastButtonState0) //logic1
  {
    lastDebounceTime0 = millis();
  }

  if ((millis() - lastDebounceTime0) > debounceDelay0) 
  {
    if (reading0 != buttonState0) 
    {
      buttonState0 = reading0;
      if (buttonState0 == LOW) 
      {
        M = !M;
        Serial.println(M);
      }
    }
  }
  lastButtonState0 = reading0;
// led báo mode hiển thị hiện tại 
  if ( M == 0 )
  {
    digitalWrite(A0,HIGH);
    digitalWrite(13,LOW);
    lcd.setCursor(0,0);//col, row

    lcd.print("Nhiet do say:");
    lcd.setCursor(2,1);
    lcd.print(Tc);
    lcd.print( (char)223);     // degree symbol so 223 lay tu bang ma ascII
    lcd.print("C");     // C character
////////////////////////////////////////////// button1: minus

  
int reading1 = digitalRead(1);
  if (reading1 != lastButtonState1) 
  {
    lastDebounceTime1 = millis();
  }

  if ((millis() - lastDebounceTime1) > debounceDelay1) 
  {
    if (reading1 != buttonState1) 
    {
      buttonState1 = reading1;
      if (buttonState1 == HIGH) 
      {
       
        z=z-1;
        if (z < 0) 
        {
          y = y - 1;
          z = 9;
        }
        if (y < 0) 
        {
          x = x - 1;
          y = 9;
        }
        if (x < 3) 
        {
          x = 5;
        }
        Tc = 10*x+y+z/10.0;
        Serial.println(Tc);
      }
    }
  }
  lastButtonState1 = reading1;
  
 /////////////////////////////////////////button2: plus
  
int reading2 = digitalRead(2);
  if (reading2 != lastButtonState2) 
  {
    lastDebounceTime2 = millis();
  }

  if ((millis() - lastDebounceTime2) > debounceDelay2) 
  {
    if (reading2 != buttonState2) 
    {
      buttonState2 = reading2;
      if (buttonState2 == LOW) 
      {
        z++;
        if(z>9)
        {
          y++;
          z=0;
        }
        if(y>9)
        {
          x++;
          y=0;
        }
        if(x>8)
        {
          x=5;
        }
        Tc = 10*x+y+z/10.0;
        Serial.println(Tc);

      }
    }
  }
  lastButtonState2 = reading2;

  }
   if ( M == 1 )
  {
    digitalWrite(A0,LOW);
    digitalWrite(13,HIGH);
  }

}
