// insérer dans pref : http://arduino.esp8266.com/stable/package_esp8266com_index.json

#include <ESP8266WiFi.h>




const char* ssid = "PORTABLE";
const char* password = "12345678";

WiFiServer server(5000);


void setup()
{
  Serial.begin(9600);
  Serial.println();

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
void loop()
{
  if (!client){
    client = server.accept();
    if (client)
    {
      Serial.println("\n[Client connected]");
    }
  }
   

  // wait for a client (web browser) to connect
  if (client)
  {
    
    // read line by line what the client (web browser) is requesting
    if (client.available())
    {
      String line = client.readStringUntil('\n');
      Serial.print(line);
      // wait for end of client's request, that is marked with an empty line
      /*if (line.length() == 1 && line[0] == '\n')
      {
        client.println(prepareHtmlPage());
        break;
      }*/
    }

/*    while (client.available()) {
      // but first, let client finish its request
      // that's diplomatic compliance to protocols
      // (and otherwise some clients may complain, like curl)
      // (that is an example, prefer using a proper webserver library)
      client.read();
    }
*/
    // close the connection:
    //client.stop();
    //Serial.println("[Client disconnected]");
  }
}
