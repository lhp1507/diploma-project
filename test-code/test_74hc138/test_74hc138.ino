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
  Serial.print(F("Monday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, LOW);
  Serial.println();
  delay(1000);
  Serial.print(F("Tuesday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, HIGH);
  Serial.println();
  delay(1000);
  Serial.print(F("Wednesday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, LOW);
  Serial.println();
  delay(1000);
  Serial.print(F("Thusday"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, HIGH);
  Serial.println();
  delay(1000);
  Serial.print(F("Friday"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, LOW);
  Serial.println();
  delay(1000);
  Serial.print(F("Saturday"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, HIGH);
  Serial.println();
  delay(1000);
  Serial.print(F("Sunday"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, LOW);
  Serial.println();
  delay(1000);
}
