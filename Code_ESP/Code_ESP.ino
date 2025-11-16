// insérer dans pref : http://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <pthread.h>
#define PORT 5000


const char* ssid = "PORTABLE";
const char* password = "12345678";
int i =0;

WiFiServer server(PORT);


SoftwareSerial esp_serial =  SoftwareSerial(13, 15); // Mettre le blanc sur 13 et vert sur 15

void setup() {
  Serial.begin(115200);
  esp_serial.begin(115200);
  pinMode(2, OUTPUT);
  Serial.setTimeout(10);
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());

}

WiFiClient client; // socket client TCP

void loop() { 

  if (!client){
    client = server.accept();
    if (client)
    {
      Serial.println("\n[Client connected];");
    }
  }
   

  // wait for a client (web browser) to connect
  if (client)
  {
    
    // read line by line what the client (web browser) is requesting
    if (client.available())
    {
      String line = client.readStringUntil('\n');
      
      if (line == "STOP"){
        Serial.println("Arret de transfert!");
        client.print("STOP");
        Serial.println("Message envoyé à la méga : /MOTORA 0;");
        esp_serial.print("/MOTORA 0;\n");
        Serial.println("Message envoyé à la méga : /MOTORB 0;");
        esp_serial.print("/MOTORB 0;\n");
        client.stop();
        Serial.println("[Client disconnected]");
        digitalWrite(2, LOW);
      }
      else if (line.startsWith("/MEGA")){
        Serial.println("Message envoyé à la méga : "+ line.substring(5));
        esp_serial.print(line.substring(5)+"\n");
      }
      
      
    }
    if(esp_serial.available()){
      String message = esp_serial.readStringUntil('\n');
      if (message.startsWith("/ORDI")){
          
          Serial.print("Message envoyé au client : " + message.substring(5) + "\n");
          client.print(message.substring(5));
        }
      }
  }

 
  digitalWrite(2, HIGH);

}
