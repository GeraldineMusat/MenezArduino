#include <WiFi.h>

#include "OneWire.h"
#include "DallasTemperature.h"

WiFiServer server(80); //Declare un web server
OneWire oneWire(23); // sur le port 23
DallasTemperature tempSensor(&oneWire);
const int ledPin = 19; // sur le port 19
const int temp = A0;

String header;
int ledState = 0; // eteinte
int sensorValue;
float t;

const char* ssid = "LicorneDeSubstitution"; // Nom de la connexion WIFI
const char* password= "ordinateur"; // Mot de passe de la connexion WIFI

               
void print_status(){
  Serial.print("WiFi connected \n");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
  Serial.print("MAC address: ");
  Serial.print(WiFi.macAddress());
  Serial.print("\n"); 
}

void connect_wifi(){
 
 Serial.println("Connecting Wifi...");
 WiFi.begin(ssid, password);
 while(WiFi.status() != WL_CONNECTED){
   Serial.println("Attempting to connect ..");
   delay(1000);
 }
 
 Serial.print("Connected to local Wifi\n");  
 print_status();
}

// this method makes a HTTP reply:
void httpReply(WiFiClient client) {
   client.println("HTTP/1.1 200 OK");
   client.println("Content-Type: text/html");
   client.println("Connection: close");  // the connection will be closed after completion of the response
   client.println("Refresh: 2");         // refresh the page automatically every 5 sec
   client.println();
   client.println("<!DOCTYPE HTML>");
   client.println("<html lang=\"fr\">");
   client.println("<head>");    
   client.println("<title>ESP32</title>"); 
   client.println("<meta charset=\"utf-8\"/>"); 
   client.println("</head>");  
   client.println("<body>");  
   client.println("<h1>ESP32 Devkitc</h1>");  
   client.println("<h2>Web Control of LeD, Photoresistor, Temperature</h2>");  
   client.println("<br>");  
   client.println("<p>Object IP : ");  
   client.println(WiFi.localIP());
   client.println("<p/>");
   client.println("<p>Valeur de la luminosite : ");  
   client.println(sensorValue, DEC); 
   client.println("<p/>");
   client.println("<p>Valeur de la temperature : ");  
   client.println(t); 
   client.println("<p/>");
   client.println("<br/>");
   if(ledState == 0) { // Si led eteinte
      client.println("<a href=\"/on\"><input class=\"on\" type=\"button\" value=\"LED ON\"></a>");
   } else {
      client.println("<a href=\"/off\"><input class=\"off\" type=\"button\" value=\"LED OFF\"></a>");
   } 
   client.println("</body>");     
   client.println("</html>"); 
}

void setup(){ 
  Serial.begin(9600);
  tempSensor.begin();
  pinMode(ledPin, OUTPUT);
  delay(1);

  connect_wifi(); // Connexion Wifi  
  server.begin(); // Lancement du serveur
}

void loop() {
  sensorValue = analogRead(temp);
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        Serial.write(c); // Pour voir sur la console ce que l'on a recu
        if (c == '\n' && currentLineIsBlank) { // send a standard http response header
          
          //recupere le header
          if(header.indexOf("GET /on") >= 0){
            digitalWrite(ledPin, HIGH);
            ledState = 1; // allumee
          }
          if(header.indexOf("GET /off") >= 0){
            digitalWrite(ledPin, LOW);
            ledState = 0; // eteinte
          }
          
          sensorValue = analogRead(temp);
          tempSensor.requestTemperaturesByIndex(0);
          t = tempSensor.getTempCByIndex(0);
          
          httpReply(client);
          break;
        }
        if (c == '\n') { // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') { // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(100); // ms

    // close the connection :
    header = "";
    client.stop();
    Serial.println("client disconnected");
  }
}
