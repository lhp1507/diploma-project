/*
 * NOTE: OUTPUT {Y0;...;Y7} active LOW
 * E'1;E'2 active LOW
 * E3 active HIGH
 */

#define inputC D6
#define inputB D7
#define inputA D8

void setup()
{
  pinMode(inputC, OUTPUT);
  pinMode(inputB, OUTPUT);
  pinMode(inputA, OUTPUT);

  Serial.begin(115200);
}

void loop()
{
  if(Serial.available()){
    String st = Serial.readString();
  
  
  if(st == "2")
  {
  Serial.print(F("Monday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, LOW);
  Serial.println();
  }
  if(st == "tue")
  {
  Serial.print(F("Tuesday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, HIGH);
  Serial.println();
  }
  if(st == "wed")
  {
  Serial.print(F("Wednesday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, LOW);
  Serial.println();
  }
  if(st == "thu")
  {
  Serial.print(F("Thusday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, HIGH);
  Serial.println();
  }
  if(st == "fri")
  {
  Serial.print(F("Friday"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, LOW);
  Serial.println();
  }
  if(st == "sat")
  {
  Serial.print(F("Saturday"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, HIGH);
  Serial.println();
  }
  if(st == "sun")
  {
  Serial.print(F("Sunday"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, LOW);
  Serial.println();
}}

}
