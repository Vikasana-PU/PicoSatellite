/*
  ------------------------
  Author : Sukarna Jana
  Date   : 02/06/2022
  Title  : Pico Satterlite
  ------------------------

  Pin Connection:
  RTC Module
  Micro SD Card Module
  MQ
  LDR


  ThingSpeak config -----
  Field 1 = Temperature
  Field 2 = Humidity
  Field 3 = Air Quality
  Field 4 = Light Intensity
  Field 5 = Air Pressure

  PinOut Connection------
  DHT11 --> ESP8266-12E
   VCC         3.3V
   OUT         D3
   GND         GND
  BMP180 --> ESP8266-12E
   VIN         3.3V
   GND         GND
   SCL         D1
   SDA         D2
*/

#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include "DHT.h"
#include "MQ135.h"

#define dhtType DHT11
#define dhtPin D3
#define statusLed D0
#define airQ_sunI A0
#define controlMQ D7
#define controlLDR D8
WiFiClient  client;
Adafruit_BMP085 bmp;

const char* ssid = "LocalNetwork";
const char* password = "sukarna jana";

unsigned long myChannelID = 1760052;
const char * myWriteAPIKey = "Y24XIRWTTTL3P61X";

// For LDR Luximeter 
#define VIN 3.3 
#define R 220 
int LDR_Val;

int humi, temp;
int bmpT, bmpP;
int airQ;
int luxV;

DHT dht(dhtPin, dhtType);

void setup() {
  dht.begin();
  Serial.begin(115200);
  ThingSpeak.begin(client);

  pinMode(controlMQ, OUTPUT);
  pinMode(controlLDR,OUTPUT);

  digitalWrite(controlMQ,0);
  digitalWrite(controlLDR,0);
  
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, 1);

  // ---- CHECK INTERNET CONNECTION- //
  WiFi.begin(ssid, password);       
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 
  
  // ---- CHECK ALL THE SENSORS ---- //
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    digitalWrite(statusLed, 0);
    while (1) {}
  }
  if (int(dht.readHumidity()) > 100 || int(dht.readTemperature()) > 150) {
    Serial.println("There is Some issue with DHT Sensor");
    digitalWrite(statusLed, 0);
  }

  Serial.println("READY TO START");
  digitalWrite(controlMQ,1);
  delay(3000);
  digitalWrite(controlMQ,0);
}

void loop() {

    // ------------- TEMP & HUMI ------------- //
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    delay(500);
    // ------------- TEMP & PA --------------- //
    bmpT = bmp.readTemperature();
    bmpP = bmp.readPressure();
    delay(500);
    // ------------- PPM --------------------- //
    digitalWrite(controlMQ,1);
    delay(1000);
    MQ135 gasSensor = MQ135(airQ_sunI);
    airQ = gasSensor.getPPM();
    digitalWrite(controlMQ,0);
    // ------------- LUX --------------------- //
    digitalWrite(controlLDR,1);
    delay(1000);
    LDR_Val = analogRead(airQ_sunI);
    float Vout = float(LDR_Val) * (VIN / float(1023));
    float RLDR = (R * (VIN - Vout))/Vout;
    luxV = 500/(RLDR/1000);
    digitalWrite(controlLDR,0);
    // ------------- PRINT ALL DATA ---------- //
    Serial.println("---- Data ----");
    Serial.print("Humidity :");
    Serial.print(humi);
    Serial.print(" %\nTemperature :");
    Serial.print(temp);
    Serial.print(" *C\nBMP180 Temp. :");
    Serial.print(bmpT);
    Serial.print(" *C\nPressure :");
    Serial.print(bmpP);
    Serial.print(" Pa\nAir Quality :");
    Serial.print(airQ);
    Serial.print("PPM\nLUX :");
    Serial.print(luxV);
    Serial.print("LUX\n");

    ThingSpeak.setField(1,temp);
    ThingSpeak.setField(2,bmpT);
    ThingSpeak.setField(3,humi);
    ThingSpeak.setField(4,bmpP);
    ThingSpeak.setField(5,airQ);
    ThingSpeak.setField(6,luxV);
    int x = ThingSpeak.writeFields(myChannelID,myWriteAPIKey);

    if(x == 200){
      Serial.println("All The Data Has been sended Successfully");
    }
    else{
      Serial.println("Some Error has happend");
      Serial.print("Status : ");
      Serial.println(x);
    }
    
    digitalWrite(statusLed, 1);
    delay(100);
    digitalWrite(statusLed, 0);
    delay(100);
    digitalWrite(statusLed, 1);
    delay(100);

    delay(5000);
}
