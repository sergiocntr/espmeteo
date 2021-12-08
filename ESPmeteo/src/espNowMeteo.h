/****************************************************************************************************************************************************
 *  TITLE: ESP-NOW Getting Started Examples
 *
 *  By Frenoy Osburn
 *  YouTube Video: https://youtu.be/_cNAsTB5JpM
 ****************************************************************************************************************************************************/

 /********************************************************************************************************************
  * Please make sure that you install the board support package for the ESP8266 boards.
  * You will need to add the following URL to your Arduino preferences.
  * Boards Manager URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
 ********************************************************************************************************************/
 
 /********************************************************************************************************************
 *  Board Settings:
 *  Board: "WeMos D1 R1 or Mini"
 *  Upload Speed: "921600"
 *  CPU Frequency: "80MHz"
 *  Flash Size: "4MB (FS:@MB OTA:~1019KB)"
 *  Debug Port: "Disabled"
 *  Debug Level: "None"
 *  VTables: "Flash"
 *  IwIP Variant: "v2 Lower Memory"
 *  Exception: "Legacy (new can return nullptr)"
 *  Erase Flash: "Only Sketch"
 *  SSL Support: "All SSL ciphers (most compatible)"
 *  COM Port: Depends *On Your System*
 *********************************************************************************************************************/
 #include<ESP8266WiFi.h>
#include<espnow.h>
#include<cxonfig.h>
#define MY_NAME         "METEO_NODE"
#define MY_ROLE         ESP_NOW_ROLE_CONTROLLER         // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE   ESP_NOW_ROLE_SLAVE              // set the role of the receiver
#define WIFI_CHANNEL    6
//EC:FA:BC:0D:5A:52
uint8_t receiverAddress[] = {0xEC, 0xFA, 0xBC, 0x0D, 0x5A, 0x52};   // please update this with the MAC address of the receiver

// struct __attribute__((packed)) dataPacket {
//   int sensor1;
//   int sensor2;
//   float sensor3;
// };

void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
//   if(transmissionStatus == 0) {
//     Serial.println("Data sent successfully");
//   } else {
//     Serial.print("Error code: ");
//     Serial.println(transmissionStatus);
//   }
    return;
}

void invioNow() {
//   Serial.begin(115200);     // initialize serial port
  
//   Serial.println();
//   Serial.println();
//   Serial.println();
//   Serial.print("Initializing...");
//   Serial.println(MY_NAME);
//   Serial.print("My MAC address is: ");
//   Serial.println(WiFi.macAddress());
  mDataPack.battery=currmet.battery;
  mDataPack.externalPressure=currmet.externalPressure;
  mDataPack.humidityBMP=currmet.humidityBMP;
  mDataPack.temperatureBMP=currmet.temperatureBMP;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

//   if(esp_now_init() != 0) {
//     Serial.println("ESP-NOW initialization failed");
//     return;
//   }
  esp_now_init();
  esp_now_set_self_role(MY_ROLE);   
  esp_now_register_send_cb(transmissionComplete);   // this function will get called once all data is sent
  esp_now_add_peer(receiverAddress, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

  //Serial.println("Initialized.");

  
  esp_now_send(receiverAddress, (uint8_t *) &mDataPack, sizeof(mDataPack));

  
}