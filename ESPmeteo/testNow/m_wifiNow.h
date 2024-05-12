#pragma once
#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h> 
  #include <ESP8266HTTPClient.h>
  #include <ESP8266httpUpdate.h>
#endif //ESP8266
// QUI VANNO LIBRERIE SPECIFICHE PER ESP 32
#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
#endif  //ESP32
#include <PubSubClient.h>
//#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>
//#include <topic.h>
//#include <myIP.h>
//#include "password.h"
namespace wifiK{
WiFiClient mywifi;
WiFiClient c;
PubSubClient client(c);
const uint8_t versione =3;
const char* myId ="caminetto";
uint8_t mqttOK=0;
const uint32_t wifi_check_time=5000L;
uint32_t wifi_initiate =0;
//uint8_t wifi_reconnect_tries = 0;
float t_sondaK ;
float t_sondaDS ;
float OutPutPID;

//QUESTO CODICE NON BLOCCANTE LO UTILIZZO AL POSTO DEL DELAY
void smartDelay(uint32_t ms){
  uint32_t start = millis();
  do
  {
    client.loop();
    delay(10);
  } while (millis() - start < ms);
}

//ROUTINE PER AGGIORNAMENTO FIRM VIA WB
uint8_t checkForUpdates(uint8_t FW_VERSION) {
  uint8_t check=0;
  String fwURL = String( fwUrlBase );
  fwURL.concat( myId ); //QUI DA VEDERE CON PERCORSO SERVER
  String fwVersionURL = fwURL;
  fwVersionURL.concat( "/version.php" );
  String fwImageURL = fwURL;
  fwImageURL.concat( "/firmware.bin" );
  delay(100);
  WiFiClient myUpdateConn;
  HTTPClient http;
  http.begin( myUpdateConn, fwVersionURL );
  int httpCode = http.GET();
  if( httpCode == 200 ) {
    String newFWVersion = http.getString();
    int newVersion = newFWVersion.toInt();
    if( newVersion > FW_VERSION ) {
    //  //Serial.println( "Preparing to update" );
      t_httpUpdate_return ret = ESPhttpUpdate.update(myUpdateConn , fwImageURL );
      switch(ret) {
        case HTTP_UPDATE_FAILED:
          check=1;
          //Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          check=2;
          //Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          //Serial.println("[update] Update ok."); // may not called we reboot the ESP
          break;
      }
    }
    else {
      check=0;
      //Serial.println( "Already on latest version" );
    }
  }
  else {
    //Serial.print( "Firmware version check failed, got HTTP response code " );
    //Serial.println( httpCode );
    check= httpCode;
  }
  http.end();
  myUpdateConn.stop();
  return check;
}

//CALLBACK QUANDO IL MODULO RICEVE VIA MQTT QUALCHE MESSAGGIO =IN QUESTO CASO SOLO LA RICHIESTA DI PROCEDERE ALL'AGGIORNAMENTO FIRM)
void callback(char* topic, byte* payload, unsigned int length) {
  byte mioDato = payload[0];
  if(strcmp(topic,updateTopic) == 0){
    delay(10);
    if(mioDato==52){ //
      client.publish(logTopic, "aggiornamento Caminetto");
      delay(10);
      uint8_t miocheck = 0;
      miocheck = checkForUpdates(versione);
      switch(miocheck) {
        case 1:
          client.publish(logTopic, "HTTP_UPDATE_FAIL"); 
          break;
        case 2:
          client.publish(logTopic, "HTTP_UPDATE_NO_UPDATES");
          break;
        case 0:
          client.publish(logTopic, "Already on latest version" );
          break;
        default:
          //client.publish(logTopic, "Firmware version check failed, got HTTP response code " + String(miocheck));
          break;
      }
    }
  }
}

//  ROUTINE PER CREARE JSON DA MANDARE VIA MQTT
void sendThing() {
  StaticJsonDocument<200> doc;
  delay(10);
  doc["topic"] = "Cami";
  delay(10);
  doc["PID"] = OutPutPID;
  delay(10);
  doc["K"] = t_sondaK;
  delay(10);
  doc["DS"] = t_sondaDS;
  char buffer[256];
  delay(10);
  size_t n = serializeJson(doc, buffer);
  delay(10);
  mqttOK = client.publish(casaSensTopic, buffer,n);
  //delay(10);
  //Serial.println(mqttOK);
  smartDelay(100);
  //serializeJsonPretty(doc, Serial);
  //if(mqttOK){Serial.println("OK!");}
}

//QUESTO FA LAMPEGGIARE IL LED PER QUALCHE DEBUG...
void blinkLed(uint8_t volte){
  for (uint8_t i = 0; i < volte; i++)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
  }
}

//QUESTO SOTTOSCRIVE MQTT AI TOPIC 
void reconnect() {
  smartDelay(50);
  client.publish(logTopic, "Caminetto connesso");
  delay(10);
  client.subscribe(systemTopic);
  delay(10);
  mqttOK=client.subscribe(updateTopic);
  smartDelay(50);
}

//QUESTO MANDA DEI MESSAGGI MQTT PER DEBUG
void sendAlarm(uint8_t alNr) {
  smartDelay(50);

  String s;
  switch (alNr)
  {
  case 1:
    s = "SHORT TO GROUND";
    break;
  case 2:
    s = "SHORT TO VCC";
    break;
  case 3:
    s = "OPEN CIRCUIT";
    break;
  case 4:
    s = "GENERIC ERROR";
    break;
  case 5:
    s = "NO READ";
    break;
  case 6:
    s = "NO COMMUNICATION";
    break;  
  default:
    break;
  }
  smartDelay(50);
  client.publish(logTopic, "Caminetto errore TC");
  smartDelay(250);
  mqttOK=client.publish(logTopic, s.c_str() );
  smartDelay(50);
}

void wifiOFF(){
  WiFi.disconnect(true);
  delay(10);
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  WiFi.forceSleepBegin();
}

//MANDO IL WEMOS A NANNA PER UN PO
void adessoDormo(){
  client.disconnect();
  delay(10);
  wifiK::wifiOFF();
  delay(10);
  system_deep_sleep_set_option(2);
  system_deep_sleep_instant(240000*1000);
  ESP.reset();
}



//ROUTINE PER CONNESSIONE WIFI
void setupWifi(){
  WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.setOutputPower(14);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  delay(10);
  WiFi.hostname(myId);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.forceSleepWake();
  delay(10);
  WiFi.config(ipCaminetto, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  delay(10);
  WiFi.begin(ssid, password);
}

//ROUTINE PER CONNESSIONE MQTT
void startMqtt()
{
  String clientId = String("cam");
  clientId += String(random(0xffff), HEX);
  delay(10);
  //randomSeed(micros());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(100);
  client.connect(clientId.c_str(),mqttUser,mqttPass);
  delay(10);
  wifi_initiate = millis();
  while (!client.connected()) {
    if ((millis() - wifi_initiate) > wifi_check_time) {
      adessoDormo();
      //dopo c'e' il restart
    }
    delay(500);
  } 
  reconnect();
  if(mqttOK){blinkLed(3);}
}

//
void connWifi(){
  wifi_initiate = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > wifi_check_time) {
      adessoDormo();
      //dopo c'e' il restart
    }
    delay(500);
  }
  blinkLed(2);
  
}
}
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/?s=esp-now
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based on JC Servaye example: https://https://github.com/Servayejc/esp8266_espnow
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t channel = 1;
int readingId = 0;
int id = 2;

unsigned long currentMillis = millis(); 
unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

uint8_t broadcastAddressX[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

enum PairingStatus {PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED, };
PairingStatus pairingStatus = PAIR_REQUEST;

enum MessageType {PAIRING, DATA,};
MessageType messageType;

// Define variables to store DHT readings to be sent
float temperature;
float humidity;

// Define variables to store incoming readings
float incomingTemp;
float incomingHum;
int incomingReadingsId;

// Updates DHT readings every 10 seconds
//const long interval = 10000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  uint8_t msgType;
  uint8_t id;
  float temp;
  float hum;
  unsigned int readingId;
} struct_message;

typedef struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;

// Create a struct_message called myData
struct_message myData;
struct_message incomingReadings;
struct_pairing pairingData;

#define BOARD_ID 2
unsigned long start;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

void printIncomingReadings(){
  // Display Readings in Serial Monitor
  Serial.println("INCOMING READINGS");
  Serial.print("Temperature: ");
  Serial.print(incomingTemp);
  Serial.println(" ºC");
  Serial.print("Humidity: ");
  Serial.print(incomingHum);
  Serial.println(" %");
  Serial.print("Led: ");
  Serial.print(incomingReadingsId);
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  Serial.print("Size of message : ");
  Serial.print(len);
  Serial.print(" from ");
  printMAC(mac);
  Serial.println();
  uint8_t type = incomingData[0];
  switch (type) {
  case DATA :  
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.print(len);
    Serial.print(" Data bytes received from: ");
    printMAC(mac);
    Serial.println();
    incomingTemp = incomingReadings.temp;
    incomingHum = incomingReadings.hum;
    printIncomingReadings();
    
    if (incomingReadings.readingId % 2 == 1){
      digitalWrite(LED_BUILTIN, LOW);
    } else { 
      digitalWrite(LED_BUILTIN, HIGH);
    }
    break;

  case PAIRING:
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    if (pairingData.id == 0) {                // the message comes from server
      Serial.print("Pairing done for ");
      printMAC(pairingData.macAddr);
      Serial.print(" on channel " );
      Serial.print(pairingData.channel);    // channel used by the server
      Serial.print(" in ");
      Serial.print(millis()-start);
      Serial.println("ms");
      //esp_now_del_peer(pairingData.macAddr);
      //esp_now_del_peer(mac);
      esp_now_add_peer(pairingData.macAddr, ESP_NOW_ROLE_COMBO, pairingData.channel, NULL, 0); // add the server to the peer list 
      pairingStatus = PAIR_PAIRED ;            // set the pairing status
    }
    break;
  }  
}

void getReadings(){
  // Read Temperature
  temperature = 22.5;
  humidity = 55.5;
}

PairingStatus autoPairing(){
  switch(pairingStatus) {
  case PAIR_REQUEST:
    Serial.print("Pairing request on channel "  );
    Serial.println(channel);
  
    // clean esp now
    esp_now_deinit();
    WiFi.mode(WIFI_STA);
    // set WiFi channel   
    wifi_promiscuous_enable(1);
    wifi_set_channel(channel);
    wifi_promiscuous_enable(0);
    //WiFi.printDiag(Serial);
    WiFi.disconnect();

    // Init ESP-NOW
    if (esp_now_init() != 0) {
      Serial.println("Error initializing ESP-NOW");
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    // set callback routines
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    // set pairing data to send to the server
    pairingData.id = BOARD_ID;     
    pairingData.channel = channel;
    previousMillis = millis();
    // add peer and send request
    Serial.println(esp_now_send(broadcastAddressX, (uint8_t *) &pairingData, sizeof(pairingData)));
    pairingStatus = PAIR_REQUESTED;
    break;

  case PAIR_REQUESTED:
    // time out to allow receiving response from server
    currentMillis = millis();
    if(currentMillis - previousMillis > 100) {
      previousMillis = currentMillis;
      // time out expired,  try next channel
      channel ++;
      if (channel > 11) {
        channel = 0;
      }
      pairingStatus = PAIR_REQUEST; 
    }
    break;

  case PAIR_PAIRED:
    //Serial.println("Paired!");
    break;
  }
  return pairingStatus;
} 
void setup() {
  // Init Serial Monitor
  Serial.begin(74880);
  pinMode(LED_BUILTIN, OUTPUT);
  // Init DHT sensor
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  pairingData.id = 2;
}
 
void loop() { 
  if (autoPairing() == PAIR_PAIRED) { 
    static unsigned long lastEventTime = millis();
    static const unsigned long EVENT_INTERVAL_MS = 10000;
    if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
      Serial.print(".");
      getReadings();

      //Set values to send
      myData.msgType = DATA;
      myData.id = 2;
      myData.temp = temperature;
      myData.hum = humidity;
      myData.readingId = readingId ++;
      
      // Send message via ESP-NOW to all peers 
      esp_now_send(pairingData.macAddr, (uint8_t *) &myData, sizeof(myData));
      lastEventTime = millis();
    }
  }
}