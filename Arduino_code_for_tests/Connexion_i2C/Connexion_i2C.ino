#include <Wire.h>


void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println();
  Serial.println("Mega ok");

}

void loop() {
  Serial.println("transmission début");
  Wire.beginTransmission(1);   //Transmission à l'esclave d'adresse 1
  Wire.write("BONJOUR! ");
  Wire.endTransmission();
  Serial.println("transmission finie");

  delay(500); //Attente suffisante à la réponse complète de l'esclave

  Wire.requestFrom(1, 6); //Lire 6 octets de l'esclave d'adresse 1
  while (Wire.available()) {
    char c = Wire.read();
    Serial.print(c);
  }
}
