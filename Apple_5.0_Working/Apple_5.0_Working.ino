#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(-1);

const unsigned long DisoffInterval = 40000;
unsigned long previousTime = 0;

#define BLYNK_PRINT Serial
#define SET_PIN 0
#define DHTPIN 2
#define DHTTYPE DHT22


DHT dht(DHTPIN, DHTTYPE);
SimpleTimer timer;

char blynk_token[40] = "";
bool shouldSaveConfig = false;


void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  pinMode(SET_PIN, INPUT_PULLUP);
  display.clearDisplay();   
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(28, 2);
  display.println("City Greens");
  display.setCursor(2, 14);
  display.println("IoT Temp/Humi Logger");
  display.display();
  delay(2000);
  display.clearDisplay();

  dht.begin();
  //timer.setInterval(1000L, sendSensor);
  Serial.println();
  pinMode(SET_PIN, INPUT_PULLUP);
  delay(1000);

  //read configuration from FS json
  Serial.println("mounting FS...");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(3, 3);
  display.println("Conneecting... to" );
  display.setCursor(14, 12);
  display.println(" Saved Network ");
  display.display();


  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);

  if (digitalRead(SET_PIN) == LOW) 
  {
    wifiManager.resetSettings();
  }

  if (!wifiManager.autoConnect("CityGreens"))  //removed ! (!wifiManager)
  {
    Serial.println("failed to connect and hit timeout");
    display.clearDisplay(); // Clear the buff
    display.setCursor(2, 0);
    display.println(F("Failed To connect"));
    display.setCursor(2, 10);
    display.println(F("Hot-Spot Started for"));
    display.setCursor(2, 19);
    display.println(F("New Network Configuration"));
    display.display();
    delay(2000); 
    display.clearDisplay(); 
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  Serial.println("wifi connected");
  display.clearDisplay(); 
  display.setCursor(0, 4);
  display.println(F("Connected to Network"));
  display.display();
  delay(500); 
  display.clearDisplay(); 
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  Serial.println();
  Serial.print("local ip : ");
  Serial.println(WiFi.localIP());
  Serial.print("Blynk Token : ");
  Serial.println(blynk_token);
  Blynk.config(blynk_token);
}

void loop() 
{
  timer.setInterval(1000L, sendSensor);
  delay(1000);
  Blynk.run();
  timer.run(); 
  
}

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  float q = map(analogRead(A0), 4000.4f, 2400.0f, 0, 100);

  Blynk.virtualWrite(V6, h);
  Blynk.virtualWrite(V5, t);
  Blynk.virtualWrite(V0, q);
  Serial.print("Temp = ");
  Serial.print(t);
  Serial.print(" Humi = ");
  Serial.println(h);
  Serial.print(" Batt = ");
  Serial.println(q);
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE);        
  display.setCursor(18, 3);           
  display.print(F("Temp = "));
  display.print(t);
  display.println(" C");
  display.setCursor(18, 13);
  display.print(F("Humi = "));
  display.print(h);
  display.println(" %");
  display.setCursor(18, 23);
  display.print(F("Batt = "));
  display.print(q);
  display.println(" %");
  display.display();
  unsigned long currentTime = millis();
  if (currentTime - previousTime >= DisoffInterval)
  {
    display.ssd1306_command(SSD1306_DISPLAYOFF); // To switch display off
  
  ESP.deepSleep(300e6);
}

void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
