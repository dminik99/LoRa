#include <Arduino.h>
#include "disk91_LoRaE5.h"

Disk91_LoRaE5 lorae5(false); // true, false whatever

uint8_t deveui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD8, 0x00, 0x38, 0x7A };
uint8_t appeui[] = { 0x26, 0xB5, 0x01, 0x49, 0xF8, 0xD8, 0x5A, 0x3E, 0x91, 0xBD, 0x4E, 0xE2, 0x9D, 0x04, 0xC5, 0x88 };
uint8_t appkey[] = { 0x3D, 0xB1, 0x7C, 0x15, 0x14, 0x0E, 0xDB, 0x84, 0xE2, 0xE1, 0x08, 0xAA, 0x8B, 0x74, 0x23, 0x01 };

void setup() {

  Serial.begin(9600);
  uint32_t start = millis();
  while ( !Serial && (millis() - start) < 1500 );  // Open the Serial Monitor to get started or wait for 1.5"

  // init the library, search the LORAE5 over the different WIO port available
  if ( ! lorae5.begin(DSKLORAE5_HWSEARCH) ) {
    Serial.println("LoRa E5 Init Failed");
    while(1); 
  }

  // Setup the LoRaWan Credentials
  if ( ! lorae5.setup(
          DSKLORAE5_ZONE_EU868,     // LoRaWan Radio Zone EU868 here
          deveui,
          appeui,
          appkey
       ) ){
    Serial.println("LoRa E5 Setup Failed");
    while(1);         
  }

}

void loop() {
  static uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 }; 

  // Send an uplink message. The Join is automatically performed
  if ( lorae5.send_sync(
        1,              // LoRaWan Port
        data,           // data array
        sizeof(data),   // size of the data
        false,          // we are not expecting a ack
        7,              // Spread Factor
        14              // Tx Power in dBm
       ) 
  ) {
      Serial.println("Uplink done");
      if ( lorae5.isDownlinkReceived() ) {
        Serial.println("A downlink has been received");
        if ( lorae5.isDownlinkPending() ) {
          Serial.println("More downlink are pending");
        }
      }
  }
  delay(30000);
    
}