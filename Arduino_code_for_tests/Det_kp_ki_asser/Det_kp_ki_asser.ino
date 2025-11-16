#include <Encoder.h>
#include <string.h>
#define PINA 47
#define PWMA 45
#define PINB 37
#define PWMB 7
#define FACTEUR_A 224 // 238 signaux/cm
#define FACTEUR_B 14
#define VITESSE_MAX_MOTEUR_A 255
#define VITESSE_MAX_MOTEUR_B 128

Encoder encoder_motor_A(2, 5);
Encoder encoder_motor_B(3, 6);

const float kp_moteur_A = 1; // voir moodle mecatro pour déterminer proprement kp et ki

const float kp_moteur_B = 0.57; // voir moodle mecatro pour déterminer proprement kp et ki

float consigne_en_cm_moteur_A = 0.5;
int consigne_moteur_A = consigne_en_cm_moteur_A*FACTEUR_A;
long pos_moteur_A = 0;

int erreur_moteur_A = 0 ;
int commande_moteur_A = 0;


float consigne_en_cm_moteur_B = 0;
int consigne_moteur_B = consigne_en_cm_moteur_B*FACTEUR_B;
long pos_moteur_B = 0;

int erreur_moteur_B = 0 ;
int commande_moteur_B = 0;

bool activer_asser_A = false;
bool activer_asser_B = false;

int t0 = 0;
unsigned long t_envoie = 0;
unsigned long t1;
int dt;  //calcul de dt

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PINA, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PINB, OUTPUT);
  pinMode(PWMB, OUTPUT);
  calibration();
  delay(0.5);
}

void loop() {

  pos_moteur_A = encoder_motor_A.read();
  pos_moteur_B = encoder_motor_B.read();

  t1 = millis(); //Temps en secondes donc /1000
  //dt = t1 - t0 ;

  if (t1 - t_envoie > 300) {
    Serial.print("/ORDI /POSA " + String(pos_moteur_A) + "\n");
    delay(100);
    Serial.print("/ORDI /POSB " + String(pos_moteur_B) + "\n");
    t_envoie = t1;
  }

  if (activer_asser_A) {
    erreur_moteur_A = consigne_moteur_A - pos_moteur_A;
    //
    /*if (abs(erreur_moteur_A) < 200) {
      erreur_cumulee_moteur_A += erreur_moteur_A;
      }

      if (abs(ki_moteur_A * erreur_cumulee_moteur_A * dt) > VITESSE_MAX_MOTEUR_A / 2) {
      erreur_cumulee_moteur_A = 0;
      }*/
    commande_moteur_A = kp_moteur_A * erreur_moteur_A; //+ ki_moteur_A * erreur_cumulee_moteur_A * dt;
    //Serial.println(commande_moteur_A);
    moteur_A(commande_moteur_A);
  }

  if (activer_asser_B) {
    erreur_moteur_B = consigne_moteur_B - pos_moteur_B;

    /*if (abs(erreur_moteur_B) < 200) {
      erreur_cumulee_moteur_B += erreur_moteur_B;
    }

    if (abs(ki_moteur_B * erreur_cumulee_moteur_B * dt) > VITESSE_MAX_MOTEUR_B / 2) {
      erreur_cumulee_moteur_B = 0;
    }*/

    commande_moteur_B = kp_moteur_B * erreur_moteur_B ;//+ki_moteur_B * erreur_cumulee_moteur_B * dt;

    moteur_B(commande_moteur_B);
  }


  t0 = t1;
}


void calibration() { // positionnement du moteur à 0
  int t_actuel = 0;
  int t_depart = 0;
  long pos_actuel = 0;
  bool reset_time = false;
  long pos_precedent = 0;
  bool verif_temps = false;

  encoder_motor_B.write(0);
  while (not reset_time) {
    moteur_A(-150); // avance jusqu'à ne plus pouvoir
    t_actuel = millis();
    pos_actuel = encoder_motor_A.read();
    Serial.println(pos_actuel);
    if (pos_actuel == pos_precedent) {
      if (not verif_temps) {
        t_depart = millis(); // on lance le chrono de vérification de position constante
        verif_temps = true;
      }
      if (t_actuel - t_depart > 100) { // si la position reste la même pendant plus de 50 ms
        encoder_motor_A.write(0);
        reset_time = true; // calibration terminée
      }
    }
    else {
      verif_temps = false; // si la position n'est pas égale pendant au moins 50 ms alors on arrête le chrono et on continue
    }
    pos_precedent = pos_actuel;
  }
  moteur_A(0);
}

int distance_cm(float distance) { // convertit nb de signaux en distance 6900 signaux pour 29cm
  int pos = distance * FACTEUR_A;
  return (pos); // il suffit d'inverser les pin 2 et 3 popur mettre en positif
}

int verification_consigne(int consigne_en_cm) {
  if (consigne_en_cm > 29) {
    consigne_en_cm = 29;
  }
  else if (consigne_en_cm < 0) {
    consigne_en_cm = 0;
  }

  return distance_cm(consigne_en_cm);
}

void moteur_A(int vitesse) {  // vitesse de -255 à 255 moteur 12 V
  if (vitesse > 0) {
    digitalWrite(PINA, HIGH);
  }
  else {
    digitalWrite(PINA, LOW);
  }
  vitesse = abs(vitesse);
  if (vitesse > 255) {
    vitesse = 255;
  }
  analogWrite(PWMA, vitesse);
}

void moteur_B(int vitesse) {  // vitesse de -128 à 128 car moteur 6V
  if (vitesse > 0) {
    digitalWrite(PINB, HIGH);
  }
  else {
    digitalWrite(PINB, LOW);
  }
  vitesse = abs(vitesse);
  if (vitesse > 200) {
    vitesse = 200;
  }
  analogWrite(PWMB, vitesse);
}
