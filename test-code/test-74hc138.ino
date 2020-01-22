/*
 * NOTE: OUTPUT {Y0;...;Y7} active LOW
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
  delay(500);
  Serial.println(F("------ Begin ------"));
  delay(1000);
  Serial.print(F("0"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, LOW);
  Serial.println();
  
  delay(1000);
  Serial.print(F("1"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, HIGH);
  Serial.println();

  delay(1000);
  Serial.print(F("2"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, LOW);
  Serial.println();

  delay(1000);
  Serial.print(F("3"));
  digitalWrite(inputC, LOW);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, HIGH);
  Serial.println();

  delay(1000);
  Serial.print(F("4"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, LOW);
  Serial.println();

  delay(1000);
  Serial.print(F("5"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, LOW);
  digitalWrite(inputA, HIGH);
  Serial.println();

  delay(1000);
  Serial.print(F("6"));
  digitalWrite(inputC, HIGH);
  digitalWrite(inputB, HIGH);
  digitalWrite(inputA, LOW);
  Serial.println();
}
