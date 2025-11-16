#include <Encoder.h>
Encoder myEnc(2, 5);  //phototransistor 1 sur 2 et 2 sur 3 

#define direct 12
#define PWM 3 // ne pas utiliser la pin 3
#define facteur 238 // 238 signaux/cm
#define vitesse_max 128 // 128 est adapté au moteur 9V, sera a réadapter pour chaque moteur sur proto final

const float kp = 6; // voir moodle mecatro pour déterminer proprement kp et ki
const float ki = 2; // ki et kp à réadapter pour proto final

int erreur = 0 ;
long erreur_cumulee = 0 ;
int commande = 0;
float consigne_en_cm = 12;
int consigne;
long pos = 0;
long pos_precedent = 0;
long pos_actuel = 0;
int t_actuel = 0;
int t_depart = 0;
int t0 = 0;
unsigned long t1;
int dt;  //calcul de dt
bool reset_time = false;
bool verif_temps = false;
//Stockage du temps actuel t2 dans t1 pour mesure dt à la boule précédente

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  // vitesse de communication suffisante pour plus de precision
  pinMode(direct, OUTPUT);
  pinMode(PWM, OUTPUT);// direction du moteur
  calibration();
  verification_consigne(consigne_en_cm); // verification que consigne < 29cm
}

void loop() {
  pos = myEnc.read();
  Serial.print(pos);  // Affiche la position dans le moniteur série
  Serial.print(",");
  Serial.println(consigne);
  t1 = millis() * 1000; //Temps en secondes donc *1000
  dt = t1 - t0 ;
  erreur = pos - consigne;
  erreur_cumulee += erreur;
  if (abs(ki * erreur_cumulee * dt) > vitesse_max) {
    erreur_cumulee = 0;
  }
  commande = kp * erreur + ki * erreur_cumulee * dt;
  moteur(commande);
  t0 = t1;

}
int distance_cm(float distance) { // convertit nb de signaux en distance 6900 signaux pour 29cm
  int pos = distance * facteur;
  return (- pos); // il suffit d'inverser les pin 2 et 5 popur mettre en positif
}

void calibration() { // positionnement du moteur à 0
  while (not reset_time ) {
    moteur(-100); // avance jusqu'à ne plus pouvoir
    t_actuel = millis();
    pos_actuel = myEnc.read();
    Serial.println(pos_actuel);
    if (pos_actuel == pos_precedent) {
      if (not verif_temps) {
        t_depart = millis(); // on lance le chrono de vérification de position constante
        verif_temps = true;
      }
      if (t_actuel - t_depart > 50) { // si la position reste la même pendant plus de 50 ms
        myEnc.write(0);
        reset_time = true; // calibration terminée
      }
    }
    else {
      verif_temps = false; // si la position n'est pas égale pendant au moins 50 ms alors on arrête le chrono et on continue
    }
    pos_precedent = pos_actuel;
  }
  moteur(0);
}


void verification_consigne(int consigne_en_cm) {
  if (consigne_en_cm > 29) {
    consigne_en_cm = 29;
    consigne = distance_cm(consigne_en_cm);
  }
  else {
    consigne = distance_cm(consigne_en_cm);
  }
}

void moteur(int vitesse) {  // vitesse de -255 à 255
  if (vitesse > 0) {
    digitalWrite(direct, LOW);
  } else {
    digitalWrite(direct, HIGH);
  }
  vitesse = abs(vitesse);
  if (vitesse > vitesse_max) {
    vitesse = vitesse_max;
  }
  analogWrite(PWM, vitesse);
}
