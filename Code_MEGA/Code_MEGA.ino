#include <Encoder.h>
#include <string.h>
#define PINA 47
#define PWMA 45
#define PINB 37
#define PWMB 7
#define FACTEUR_A 224 // 224 signaux/cm
#define FACTEUR_B 14
#define VITESSE_MAX_MOTEUR_A 255
#define VITESSE_MAX_MOTEUR_B 200

Encoder encoder_motor_A(2, 5);
Encoder encoder_motor_B(3, 6);

float kp_moteur_A = 0.6; // voir moodle mecatro pour déterminer proprement kp et ki

float kp_moteur_B = 0.57; // voir moodle mecatro pour déterminer proprement kp et ki

float consigne_en_cm_moteur_A = 5;
int consigne_moteur_A = 0;
long pos_moteur_A = 0;

int erreur_moteur_A = 0 ;
int commande_moteur_A = 0;


float consigne_en_cm_moteur_B = 0;
int consigne_moteur_B;
long pos_moteur_B = 0;

int erreur_moteur_B = 0 ;
int commande_moteur_B = 0;

bool activer_asser_A = false;
bool activer_asser_B = false;
bool tracking = false;

int t0 = 0;
unsigned long t_envoie = 0;
unsigned long t1;
int dt;  //calcul de dt



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(PINA, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PINB, OUTPUT);
  pinMode(PWMB, OUTPUT);


  Serial1.setTimeout(10);
  calibration();
  //consigne_moteur_A = verification_consigne(consigne_en_cm_moteur_A);
  Serial.println("Bienvenue, annonce la commande");
}

void loop() {

  //Serial.print(String(pos_moteur_A) + " ; ");
  pos_moteur_A = encoder_motor_A.read();
  pos_moteur_B = encoder_motor_B.read();
  /*if (abs(pos_moteur_B) > 360){
    encoder_motor_B.write(0);
    pos_moteur_B = encoder_motor_B.read();
    }*/
  if (Serial1.available()) {

    String commande = Serial1.readStringUntil(';');
    commande.trim();
    Serial.println("commande recu : " + commande);

    if (commande.startsWith("/TR")) {
      tracking = true;
      activer_asser_A = true;
      activer_asser_B = false;
      consigne_moteur_A = pos_moteur_A;
      consigne_moteur_B = pos_moteur_B;

      kp_moteur_A = 0.5; // spécial pour le tracking
      kp_moteur_B = 0.57;

      commande = commande.substring(3);
      commande.trim();

      if (commande.startsWith("/A") and (pos_moteur_A < 29 * FACTEUR_A) and pos_moteur_A > 50 ) {
        commande = commande.substring(2);
        commande.trim();
        erreur_moteur_A = commande.toInt();
      }
      else {
        commande = commande.substring(2);
        commande.trim();
        erreur_moteur_B = commande.toInt();
      }

    }

    else if (commande.startsWith("/TL")) {
      tracking = false;
      String temp = "";
      //activer asservissement moteur a et b
      activer_asser_A = true;
      activer_asser_B = false;

      kp_moteur_A = 1.0; // spécial pour l'asservissement
      kp_moteur_B = 0.57;

      commande = commande.substring(3);
      commande.trim();

      temp = commande;

      commande = commande.substring(0, 7);
      commande.trim();
      consigne_en_cm_moteur_A = commande.toFloat();

      temp = temp.substring(7);
      temp.trim();
      consigne_en_cm_moteur_A = temp.toFloat();

      consigne_moteur_A = verification_consigne(consigne_en_cm_moteur_A);
      consigne_moteur_B = consigne_en_cm_moteur_B * FACTEUR_B;
    }


    else if (commande.startsWith("/MOTORA")) {
      tracking = false;
      activer_asser_A = false;
      activer_asser_B = false;
      commande = commande.substring(7);
      commande.trim();

      commande_moteur_A = commande.toInt();

      //Serial.print(commande_moteur_A);
      if (pos_moteur_A / FACTEUR_A < 29 - (0.01 * abs(commande_moteur_A)) and commande_moteur_A > 0) {
        moteur_A(commande_moteur_A);
      }
      else if (pos_moteur_A / FACTEUR_A > 0.01 * abs(commande_moteur_A) and commande_moteur_A < 0) {
        moteur_A(commande_moteur_A);
      }
      else {
        moteur_A(0);
      }
    }


    else if (commande.startsWith("/MOTORB")) {
      tracking = false;
      activer_asser_A = false;
      activer_asser_B = false;
      commande = commande.substring(7);
      commande.trim();

      commande_moteur_B = commande.toInt();

      //Serial.print(commande_moteur_B);
      moteur_B(commande_moteur_B);


    }

    else if (commande == "Calibration") {
      tracking = false;
      activer_asser_A = false;
      activer_asser_B = false;
      Serial1.print("/ORDI La barre est en cours de calibration\n");
      calibration();
      consigne_moteur_A = 0;
      Serial1.print("/ORDI La barre est calibrée !\n");
    }
  }



  t1 = millis(); //Temps en secondes donc /1000
  //dt = t1 - t0 ;

  if (t1 - t_envoie > 300) {
    Serial1.print("/ORDI /POSA " + String(pos_moteur_A / FACTEUR_A) + "\n");
    delay(100);
    Serial1.print("/ORDI /POSB " + String(pos_moteur_B / FACTEUR_B) + "\n");
    t_envoie = t1;
  }

  if (activer_asser_A) {
    if (not tracking) {
      erreur_moteur_A = consigne_moteur_A - pos_moteur_A;
    }
    commande_moteur_A = kp_moteur_A * erreur_moteur_A;
    moteur_A(commande_moteur_A);
  }

  if (activer_asser_B) {
    erreur_moteur_B = consigne_moteur_B - pos_moteur_B;

    commande_moteur_B = kp_moteur_B * erreur_moteur_B ;
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
  if (vitesse > VITESSE_MAX_MOTEUR_A) {
    vitesse = VITESSE_MAX_MOTEUR_A;
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
  if (vitesse > VITESSE_MAX_MOTEUR_B) {
    vitesse = VITESSE_MAX_MOTEUR_B;
  }
  analogWrite(PWMB, vitesse);
}
