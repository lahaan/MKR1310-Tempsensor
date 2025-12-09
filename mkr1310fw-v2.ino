#include <MKRWAN.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoLowPower.h>

// LoRaWAN
#define LORA_REGION EU868
const char *appEui = "";
const char *appKey = "";

// DS18B20
#define ONE_WIRE_BUS A6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LoRaModem modem;

// Send interval
uint32_t transmit_interval = 600; // Default 600 seconds

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  sensors.begin();
  
  if (!modem.begin(LORA_REGION)) {
    Serial.println("Failed to start LoRa modem!");
    while (1);
  }
  
  Serial.println("Joining LoRaWAN network via OTAA...");
  if (!modem.joinOTAA(appEui, appKey)) {
    Serial.println("OTAA join failed!");
    while (1);
  }
  Serial.println("NETWORK JOINED");
}

void loop() {
  // Read temp
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor error");
    delay(5000);
    return;
  }
  
  Serial.println("Temp: " + String(tempC) + " C");

  // Encode as int16 (temp * 100) → 2 bytes
  int16_t tempScaled = (int16_t)(tempC * 100.0f);
  uint8_t payload[2] = {
    (uint8_t)(tempScaled >> 8),   // MSB
    (uint8_t)(tempScaled & 0xFF)  // LSB
  };
  
  modem.beginPacket();
  modem.write(payload, 2);
  int status = modem.endPacket(true); // true = confirmed uplink (ACKed)
  
  if (status > 0) {
    Serial.println("Uplink sent!");
    
    // Check for downlink
    if (modem.available()) {
      uint8_t downlink[64];
      int len = modem.read(downlink, sizeof(downlink));
      Serial.println("Received downlink (" + String(len) + " bytes):");
      
      // Parse 2-byte interval (in seconds)
      if (len >= 2) {
        uint16_t new_interval = (downlink[0] << 8) | downlink[1];
        
        // Validate interval (10 seconds to 1 hour)
        if (new_interval >= 10 && new_interval <= 3600) {
          transmit_interval = (uint32_t)new_interval;
          Serial.println("Interval updated to " + String(transmit_interval) + " seconds");
        } else {
          Serial.println("Invalid interval value");
        }
      }
    }
  } else {
    Serial.println("Send failed (status: " + String(status) + ")");
  }
  
  Serial.println("Sleeping for " + String(transmit_interval) + " seconds...\n");
  LowPower.sleep(transmit_interval * 1000); // ms
}
