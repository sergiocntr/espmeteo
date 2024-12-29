#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "topic.h"
#include "password.h"
#include "myIP.h"
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
// void blinkLed(uint8_t volte,bool l){
// uint16_t tempo;
// if(l){tempo=250;}else{tempo=500;}
//   for (uint8_t i = 0; i < volte; i++)
//   {
//     digitalWrite(LED_BUILTIN,LOW);
//     delay(tempo);
//     digitalWrite(LED_BUILTIN,HIGH);
//     delay(tempo);
//   }
// }
bool  publish(const char* topic,const char* message ){
  return client.publish(topic, message,sizeof(message));
  
}
void smartDelay(unsigned long mytime){
  uint32_t adesso = millis();
  while((millis()-adesso)<mytime){
   client.loop();
    
    delay(10);
  }
}
bool reconnect() {
  delay(10);
  bool mqttOK=client.publish(logTopic, "prova connesso");
  client.loop();
	return mqttOK;
  }
void setWifiOff(){
	wifi_set_sleep_type(LIGHT_SLEEP_T);
	WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  	WiFi.disconnect(true);    // Delete SDK wifi config
  	delay(200);
  	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(100);

}
bool connectWifi(){
	WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.setOutputPower(14);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  delay(10);
  WiFi.hostname("prova");      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.forceSleepWake();
  delay(10);
  WiFi.config(ipChrono, gateway, subnet,dns1); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  delay(10);
  WiFi.begin(ssid, password);
  	delay(10);
	uint32_t wifi_initiate = millis();
  	while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - wifi_initiate) > 5000L) {
      	return false;
    	}
    delay(700);
  }
  delay(10);
  return true;
}
bool connectMQTT(){
	client.setServer(mqtt_server, mqtt_port);
  	//client.setCallback(callback);
  	String clientId = String(provaId);
  	clientId += String(random(0xffff), HEX);
	delay(100);
	client.connect(clientId.c_str(),mqttUser,mqttPass);
	uint32_t wifi_initiate = millis();
  	smartDelay(100);
	while (!client.connected()){
		if ((millis() - wifi_initiate) > 5000L) {
      	return false;
    	}
		delay(700);
  	}
	delay(100);
  	return true;
}
// void faiSampleDummy(){
// 	for (size_t i = 0; i < 4; i++)
// 	{
// 		requestSensorsValues();
// 		delay(10);
// 		storeData(i);
// 		delay(100);
// 		blinkLed(1,true);
// 		delay(2000);
// 	}
// }
uint8_t collegaScheda(){
	if(!connectWifi()) return 1;
	if(!connectMQTT()) return 2;
	if(!reconnect()) return 3;
	return 0;

}
