#define PIN 45 
#define PWM 47
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(PIN, OUTPUT);
  pinMode(PWM, OUTPUT);
  Serial1.setTimeout(10);
  
  Serial.println("Bienvenue, annonce la commande");
}

void loop() {
  // put your main code here, to run repeatedly:

  
  if (Serial1.available()) {
    String commande = Serial1.readStringUntil(';');
    Serial.println(commande);
    if (commande == "Allumer MOTOR") {
      Serial.println("ok");
        moteur(200);
    }
    else if (commande == "Eteindre MOTOR"){
      moteur(0);
    }
  }
  
  
  
}

void moteur(int vitesse) {  // vitesse de -255 à 255
  if (vitesse > 0) {
    digitalWrite(PIN, HIGH);
  } else {
    digitalWrite(PIN, LOW);
  }
  vitesse = abs(vitesse);
  if (vitesse > 255) {vitesse = 255;}
  analogWrite(PWM, vitesse);
}
