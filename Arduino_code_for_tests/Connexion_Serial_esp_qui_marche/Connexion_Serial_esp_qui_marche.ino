//débrancher le fil blanc pendant la compilation du code
void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  Serial1.setTimeout(10);
}

void loop() {
  Serial.println("ok");
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
   if (Serial.available()) {
    Serial.println(Serial.readStringUntil('/n'));
  }
}
