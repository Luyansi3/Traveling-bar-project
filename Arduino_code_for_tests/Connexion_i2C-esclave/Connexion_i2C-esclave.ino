#include <Wire.h>

void setup() {
  Wire.begin(1); //démarrer l'esclave d'adresse 1        
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  Serial.begin(115200);  
  Serial.println("test setup");         
}

void loop() {
  Serial.println("test loop ");    
  delay(500);
}

void receiveEvent(int howMany) {
  Serial.println("test receive ");  
  while (Wire.available()>0) {
    char ch = Wire.read();
    Serial.print(ch);         
  }
 
  Serial.println();      
}
void requestEvent() {
  Serial.println("test request ");  
  Wire.write("Salut\n");   //envoi de la réponse en 6 octets
}
