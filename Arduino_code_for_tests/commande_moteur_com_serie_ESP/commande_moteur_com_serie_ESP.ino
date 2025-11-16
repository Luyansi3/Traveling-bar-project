//débrancher le fil blanc pendant la compilation du code
int i =0;
void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  Serial1.setTimeout(10);
}

void loop() { 
   
  if (i == 5){
    Serial.print("Allumer MOTOR;");
  }
  if (i == 10){
    Serial.print("Eteindre MOTOR;");
  }
  if(i == 15){
    i =0;
  }
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);

  i++;
}
