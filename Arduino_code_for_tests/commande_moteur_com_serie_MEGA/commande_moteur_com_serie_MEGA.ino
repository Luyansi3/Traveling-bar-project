#define PIN 47 
#define PWM 45
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

  moteur(100);

  
  
  
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
