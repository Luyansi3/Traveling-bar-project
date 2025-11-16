#include <Encoder.h>
Encoder myEnc(2, 3);  //phototransistor 1 sur 2 et 2 sur 5

#define kp 0.25
#define kd 0.5

int direct = 45;   
int PWM = 47;      
int pin_intensite = 0;  // pin mesure intensité conso pour calibration à 0

int conso;
int erreur = 0;
long erreur_cumulee = 0;
int commande = 0;
int consigne;
long pos = 0;
float facteur;          // facteur en signaux par cm                  A DETERMINER
float longueur_voulue;  // position en cm objectif par rapport à 0
float intensite_max;  //                                               A DETERMINER

int t0 = 0;
int t2 = millis();  //Lecture du temps actuel (indice n, ici n=2)
int dt;             //calcul de dt
//Stockage du temps actuel t2 dans t1 pour mesure dt à la boule précédente

void setup() {
  Serial.begin(115200);  // vitesse de communication suffisante pour plus de precision
  pinMode(direct, OUTPUT);
  calibration();
}

void loop() {
  pos = myEnc.read();
  Serial.println(pos);
  conversion_longueur_signaux(longueur_voulue) ;  // permettera de changer la consigne en temps réel             
  unsigned long t1 = millis() * 1000;  //Temps en secondes donc *1000
  dt = t1 - t0;
  erreur = consigne - pos;
  erreur_cumulee += erreur;
  commande = kp * erreur+ kd*erreur_cumulee*dt;
  moteur(commande);
  t0 = t1;
}
void calibration() {
  conso = AnalogRead(pin_intensite);
  while (conso < intensite_max) {
    moteur(30);
  }
  moteur(0);
  myEnc.write(0);
}

void conversion_longueur_signaux(int longueur) {  // transforme longueur objectif en signaux pour la consigne
  consigne = longueur_voulue * facteur
}

void moteur(int vitesse) {  // vitesse de -255 à 255
  if (vitesse > 0) {
    digitalWrite(direct, HIGH);
  } else {
    digitalWrite(direct, LOW);
  }
  vitesse = abs(vitesse);
  if (vitesse > 128) {vitesse = 128;}  // ATTENTION moteur probablement 9v alimenté en 12 -> limitation PWM
  analogWrite(PWM, vitesse);
}
