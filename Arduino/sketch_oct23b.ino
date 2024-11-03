/*
  Send temperature and humidity data from a DHT sensor via LoRaWAN network
  This sketch demonstrates how to read from a DHT sensor and send data with the MKR WAN 1300/1310 board.
  This example code is in the public domain.
*/

#include <MKRWAN.h>
#include <DHT.h>

// LoRa modem instance
LoRaModem modem;

// Define DHT sensor type and pin
#define DHTPIN 2       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22  // DHT22 (AM2302) or DHT11

DHT dht(DHTPIN, DHTTYPE);

// ABP credentials
const char devAddr[] = "27FDF9BD";          // Device address in hexadecimal format
const char nwkSKey[] = "26B50149F8D85A3E91BD4EE29D04C588";  // Network session key (16 bytes in hex)
const char appSKey[] = "3DB17C15140EDB84E2E108AA8B742301";  
uint32_t counter = 0;

void setup() {
  // Serial port initialization
  Serial.begin(115200);
  while (!Serial);
  
  // Initialize DHT sensor
  dht.begin();

  // LoRa module initialization
  if (!modem.begin(EU868)) {
    Serial.println("- Failed to start module");
    while (1) {}
  }

  // Get version and EUI
  Serial.print("- Your module version is: ");
  Serial.println(modem.version());
  Serial.print("- Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  // Join network using ABP method
  int connected = modem.joinABP(devAddr, nwkSKey, appSKey);
  if (!connected) {
    Serial.println("- Something went wrong; are you indoor? Move near a window and retry...");
    while (1) {}
  }

  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
}

void loop() {
  // Read temperature and humidity values
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  counter++;

  uint32_t counterPayload = counter;
  
  // Send data via LoRa
  modem.beginPacket();
  modem.write((uint8_t*)&counterPayload, 4);  // Send counter as a 4-byte integer
  int errCounter = modem.endPacket(true);

  // Check if message was sent successfully
  if (errCounter > 0) {
    Serial.println("- Counter sent successfully!");
  } else {
    Serial.println("- Error sending counter value.");
  }
  // Check if any reading failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("- Failed to read from DHT sensor!");
    return;
  }

  // Print temperature and humidity to Serial
  Serial.print("- Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Prepare payload (2 bytes for each value, multiplied by 100 to send as integers)
  int16_t tempPayload = (int16_t)(temperature * 100);
  int16_t humPayload = (int16_t)(humidity * 100);
  
  // Send data via LoRa
  modem.beginPacket();
  modem.write((uint8_t*)&tempPayload, 2);  // Send temperature (2 bytes)
  modem.write((uint8_t*)&humPayload, 2);   // Send humidity (2 bytes)
  int errDHT = modem.endPacket(true);

  // Check if message was sent successfully
  if (errDHT > 0) {
    Serial.println("- Data sent successfully!");
  } else {
    Serial.println("- Error sending data.");
  }

  // Wait for downlink message
  delay(1000);
  if (!modem.available()) {
    Serial.println("- No downlink message received at this time");
    return;
  }
  //modem.sleep();
  // If there's a message available, store it
  char rcv[64];
  int i = 0;
  while (modem.available()) {
    rcv[i++] = (char)modem.read();
  }

  // Decode and show the received message
  Serial.print("- Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(rcv[j] >> 4, HEX);
    Serial.print(rcv[j] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Wait for 10 minutes before sending the next message
  delay(600000);  // 600000 ms = 10 minutes
}
