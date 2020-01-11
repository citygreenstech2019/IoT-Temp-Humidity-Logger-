/*
 Library Used
 */
#include "DHTesp.h"
#include <FirebaseArduino.h>
#include<ESP8266WiFi.h>
#include<WiFiClient.h>
#include<ESP8266WebServer.h>
#include<DNSServer.h>
#include<EEPROM.h>
#include <string.h>

/*
 Declaraations 
 */
int sizes = 0;
int sizep = 0;
int flag1 = 0;
int flag = 0;
String sssid;
String pass;
String interval;

const char* ssid = "City Greens";
const char* password = "12345678";


#define FIREBASE_HOST "iot-temp-humidity-logger.firebaseio.com"
#define FIREBASE_AUTH "CD6uZz9G6koenNtkZqHGGpiZHrvdAnzoKZyQVrkW"

#define DHT_PIN 4

DHTesp dht;

float temp=0, hum=0, batt=0;
/*
 Renaming 
 */

ESP8266WebServer server(80);
IPAddress myIP;
WiFiClient espClient;

/*
 Setup 
 */


void setup(void)
{
  dht.setup(DHT_PIN, DHTesp::DHT22);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  pinMode(13,INPUT_PULLUP);
  pinMode(2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(13),isr_flag, RISING);
  EEPROM.begin(512);
  Serial.begin(115200);
  delay(100);
  station();
}

void loop()
{
    temp = dht.getTemperature();
    hum = dht.getHumidity();
    batt = map(analogRead(A0), 4200.0f, 2900.0f, 20, 52);


    Serial.print ("Humidity: "); 
    Serial.print (hum);
    Serial.print (" %");
    Serial.print ("Temperature: "); 
    Serial.print (temp); 
    Serial.print (" °C ");
    Serial.print ("Battery:"); 
    Serial.print(batt); 
    Serial.println(" %");

    String tempName = Firebase.pushFloat("temp", temp);
    String humName = Firebase.pushFloat("hum", hum);
    String battName = Firebase.pushFloat("batt", batt);

      delay(5000);
      
      if(Firebase.failed())
      {
      Serial.println(Firebase.error());
      }
 
  if(flag==1)
  {
    Serial.println("Interrupt ho raha hai ");
    Config();
    flag=0;
  }
}

ICACHE_RAM_ATTR void isr_flag()
{
  flag = 1;
  Serial.println("INTERRUPT ENTERED");
}

void Config()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  Serial.println("Access Point Mode");
  WiFi.softAP(ssid,password);
  myIP = WiFi.softAPIP();
  Serial.println("Entered Configuration Mode");
  Serial.print("Hotspot IP:");
  Serial.println(myIP);
  server.begin();
  server.on("/", HandleClient);
  while(1)
  server.handleClient();
  Serial.println("HTML Page Started");
  station();
}

void HandleClient()
{
  String webpage;
  String IPaddress = myIP.toString();
  webpage="<!DOCTYPE html><html> <head> <title>Configuration</title> <style>body {background-color: #9E2E2E;font-family: Sans-Serif, Sans-Serif, Sans-Serif;Color: white;}.header {font-family: Consolas;color: lightgoldenrodyellow;}.main {border: 1px solid white;border-radius: 10px;padding: 30px;width: 60%;height: auto;}label {text-align: right;padding: 10px;}.textbox, .check {border-radius: 3px;border: 1px solid grey;margin: 10px;transition: all 0.5s;height: 20px;outline: none;padding: 5px;}.textbox:hover, .check:hover {box-shadow: 0px 0px 10px white;}.button {border-radius: 5px;background: #000;border: 1px solid white;padding: 5px;margin: 10px;display: block;color: white;font-size: 16px;transition: all 0.5s;padding: 5px 10px 5px 10px;align-self: center;}.button:hover {background: #fff;color: black;}input[type=checkbox] {width: 10px;height: 10px;border: 1px solid black;background: white;}</style> </head> <body> <center> <div class=\"header\"> <h1> City Greens Tech </h1> </div> <div class=\"main\"> <h3>IoT Device Configuration</h3> <hr> <form action=\"http://" + IPaddress + "\" method=\"POST\"> <table class=\"table\"> <tr> <td><label for=\"ssid\">SSID:</label></td> <td><input type=\"text\" name=\"ssid\" class=\"textbox\" placeholder=\"Enter SSID\" id=\"ssid\"></td> </tr> <tr> <td><label for=\"password\">Password: </label></td> <td><input type=\"password\" name=\"password\" class=\"textbox\" placeholder=\"Enter Password\" id=\"password\"></td> </tr><tr> <td><label for=\"Interval\">Time_Interval:</label></td> <td><input type=\"number\" name=\"Time_Interval\" class=\"textbox\" placeholder=\"Enter Time_Interval\" id=\"Time_Interval\"></td> </tr> </td> </tr> </table> <button type=\"submit\" value=\"Submit\" class=\"button\">Submit</button></form> </div> </center> </body></html>";
  server.send(200,"text/html" , webpage);
    if(server.args() > 0)
    {
      sssid = server.arg(0);
      pass = server.arg(1);
      interval = server.arg(2);

      delay(100);

      Serial.print("SSID:");
      Serial.println(sssid);
      Serial.print("Password:");
      Serial.println(pass);
      Serial.print("Interval:");
      Serial.println(interval);

     char ssid1[sssid.length() + 1];
     sssid.toCharArray(ssid1,sssid.length() + 1);
     char pass1[pass.length() + 1];
     pass.toCharArray(pass1,pass.length() + 1);
     char Interval1[interval.length() + 1];
     interval.toCharArray(Interval1,interval.length() + 1);

     Serial.println(sizeof(ssid1));
     Serial.println(sizeof(pass1));
     Serial.println(sizeof(Interval1));

    Serial.println("after Size of all");

    EEPROM.write(1,sizeof(ssid1));
    EEPROM.commit();
    EEPROM.write(2,sizeof(pass1));
    EEPROM.commit();
    EEPROM.write(3,sizeof(Interval1));
    EEPROM.commit();

    Serial.println("After all eep rom Commit");


    for (int i = 0; i < sizeof(ssid1); i++)
    { EEPROM.write(5 + i, ssid1[i]);
      Serial.print(ssid1[i]);
      EEPROM.commit();
    }

    for (int i = 0; i < sizeof(pass1); i++)
    { EEPROM.write(5 + i, pass1[i]);
      Serial.print(pass1[i]);
      EEPROM.commit();
    }

    for (int i = 0; i < sizeof(Interval1); i++)
    { EEPROM.write(5 + i, Interval1[i]);
      Serial.print(Interval1[i]);
      EEPROM.commit();
    }

     Serial.println("After all the for loops");
    ESP.restart();
    }
    
}

void station()
{
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(100);

  sizes = EEPROM.read(1);
  sizep = EEPROM.read(2);

  Serial.println("EEP ROM Reading");
  
  char ssid1[sizes + 1];
  char pass1[sizep + 1];

  Serial.println("After Size Read");
  
  for(int i =0; i<sizes + 1; i++)
  {
    ssid1[i] = EEPROM.read(5 + i);
  }

  for(int i =0; i<sizep + 1; i++)
  {
    pass1[i] = EEPROM.read(5 + i + sizes);
  }

  Serial.println("After For Loop Part");

   WiFi.begin(ssid1,pass1);

  Serial.print("Connecting to: ");
 // Serial.println(ssid1);

 // WiFi.begin(ssid1,pass1);

  while(WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      digitalWrite(2,1);
      delay(500);
      digitalWrite(2,0);
      delay(500);

        if(flag==1)
        {
        flag=0;
        Config();
        }
    }
  Serial.println(" ");
  Serial.print(F("[CONNECTED]"));
  Serial.println(ssid1);
  Serial.print("[ IP:");
  Serial.print(WiFi.localIP());
  Serial.print(" ]");
  
}
