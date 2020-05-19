int M = 0;

int buttonState0;             // the current reading from the input pin
int lastButtonState0 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime0 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay0 = 50;    // the debounce time; increase if the output flickers

void setup()
{
  Serial.begin(115200);
  
  pinMode(36, INPUT); // mode pin 
}

void loop() 
{
  int reading0 = digitalRead(36);
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
}
