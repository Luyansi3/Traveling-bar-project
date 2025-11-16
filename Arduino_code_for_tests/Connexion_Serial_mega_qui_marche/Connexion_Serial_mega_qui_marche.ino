void setup() {
  // put your setup code here, to run once:
Serial1.begin(9600);
Serial.begin(9600);
Serial1.setTimeout(10);
}

void loop() {
  if (Serial1.available()) {
    Serial.println(Serial1.readStringUntil('/n'));
    Serial1.println("retour");
  }
  

}
