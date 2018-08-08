#ifndef wifimio_h
#define wifimio_h
#include <ESP8266WiFi.h>
#include "cxonfig.h"
#include "debugutils.h"
uint8_t connLAN(){
  //uint8_t check=0;
  WiFi.mode(WIFI_STA);
	//WiFi.forceSleepWake();

	delay(100);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); // Set static IP (2,7s) or 8.6s with DHCP  + 2s on battery
  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED)  {
    if (millis() - timeout > 5000) {
      WiFi.mode(WIFI_OFF); //energy saving mode if local WIFI isn't connected
  		WiFi.forceSleepBegin();
      return 0;
    }
    DEBUG_PRINT("conn lan FAIL! ");
    ESP.wdtFeed();
    delay(500);
  }
  DEBUG_PRINT("conn lan OK! ");
  return 1;

}
uint8_t connINTERNET(WiFiClient c){
  unsigned long timeout = millis();
  while (!c.connect(host, httpPort)) {
    if (millis() - timeout > 5000) {
      DEBUG_PRINT("conn internet FAIL! ");
      //c.stop();
      return  0;
    }
    ESP.wdtFeed();
    delay(500);
  }
  return 1;
}

#endif
