// GPRS library example
// by Industrial Shields

#include <GPRS.h>
#include <SoftwareSerial.h>
#include "ThingsBoard.h"
#include <DHT.h>

//GPRS credentials
#define PIN "3112"
#define APN "wap.tmovil.cl"
#define USERNAME "wap"
#define PASSWORD "wap"
// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "pMBHiJU3IPZvVFwugfY9"
#define THINGSBOARD_SERVER  "165.22.154.109"
#define THINGSBOARD_PORT    8080
//DHT11
#define DHTPIN 3 //pin arduino
#define DHTTYPE DHT11 //tipo sensor
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

GPRSClient client;
uint8_t buffer[1024];

// Initialize ThingsBoard instance
ThingsBoardHttp tb(client, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT);

////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  Serial.begin(9600UL);
  if (!GPRS.begin()) {
    Serial.println("Impossible to begin GPRS");
    while(true);
  }

  int pinRequired = GPRS.isPINRequired();
  if (pinRequired == 1) {
    if (!GPRS.unlockSIM(PIN)) {
      Serial.println("Invalid PIN");
      while (true);
    }
  } else if (pinRequired != 0) {
    Serial.println("Blocked SIM");
    while (true);
  }

// Comenzamos el sensor DHT
  dht.begin();
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  static uint32_t lastStatusTime = millis();
  if (millis() - lastStatusTime > 5000) {
    uint8_t networkStatus = GPRS.getNetworkStatus();
    Serial.print("Status: ");
    Serial.println(networkStatus);
    lastStatusTime = millis();

    if ((networkStatus == 1) || (networkStatus == 5)) {
      int GPRSStatus = GPRS.getGPRSStatus();
      if (GPRSStatus == 0) {
        if (!GPRS.enableGPRS(APN, USERNAME, PASSWORD)) {
          Serial.println("GPRS not enabled");
        }
      } else if (GPRSStatus == 1) {
        if (GPRS.connected()) {
          int t = dht.readTemperature();
          int h = dht.readHumidity();
          if (!client.connected()) {
            static bool requestDone = false;
            if (!requestDone) {
              if (!client.connect("165.22.154.109", 8080)) {
                Serial.println("Error connecting to web");
              } else {
                  
                  //int t = dht.readTemperature();
                  //int h = dht.readHumidity();
                  Serial.println("connecting OK to web");
//                client.println("GET /index.html HTTP/1.1");
                  client.println("GET http://"+THINGSBOARD_SERVER+":"+THINGSBOARD_PORT+"/api/v1/"+TOKEN+"/attributes?clientKeys=attribute1,attribute2&sharedKeys=shared1,shared2
//                client.println("Host: www.industrialshields.com");
//                client.println("User-Agent: GPRS-PLC");
//                client.println();
                  Serial.print("Sending temperature data...");
                  Serial.println(t);
                  client.println(tb.sendTelemetryFloat("temperature", t));
                  Serial.print("Sending humidity data...");
                  Serial.println(h);
                  client.println(tb.sendTelemetryFloat("humidity", h));
                  //requestDone = true;
              }
            }
          } else if (client.available()) {
            Serial.println("HTTP response:");
            size_t len = client.read(buffer, sizeof(buffer));
            Serial.write(buffer, len);
            client.stop();
          }
        }
      }
    }
  }
}
