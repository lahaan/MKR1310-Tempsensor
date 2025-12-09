# Arduino MKR WAN 1310 LoRaWAN Temperature Sensor

A simple LoRaWAN temperature sensor using the Arduino MKR WAN 1310 and DS18B20 temperature sensor with downlink support for interval/sleep updates.

## Hardware

- Arduino MKR WAN 1310
- DS18B20 Temperature Sensor
- 4.7kΩ pull-up resistor

## Connections

```
DS18B20 Data Pin → A6 (MKR WAN 1310)
DS18B20 VCC      → 3.3V
DS18B20 GND      → GND
4.7kΩ Resistor   → Between Data and VCC
```

## Required Libraries

Install these libraries via Arduino Library Manager:

- **MKRWAN** (by Arduino)
- **OneWire** (by Paul Stoffregen)
- **DallasTemperature** (by Miles Burton)
- **ArduinoLowPower** (by Arduino)

## Configuration

Update the LoRaWAN credentials in the code:

```cpp
const char *appEui = "0000000000000000";  // Your App EUI
const char *appKey = "0000000000000000"; // Your App Key (16 bytes)
```

Change the region if needed:

```cpp
#define LORA_REGION EU868
```

## About code

### Temperature Sensing
- Reads DS18B20 sensor every interval
- Encodes temperature as int16 (temp × 100) in 2 bytes
- Example: 23.45°C -> `0x0929` (2341 in decimal)

### LoRaWAN Communication
- OTAA (Over-The-Air Activation)
- Confirmed uplinks (with ACK)
- Default interval: 600 seconds (10 minutes)

### Downlink Support
The device listens for downlinks after each uplink to update the transmission interval.

**Downlink Format (Port 1):**
- 2 bytes: New interval in seconds (big-endian)
- Valid range: 10-3600 seconds (10 seconds to 1 hour)

**Example RPC payload JavaScript snippet to send a downlink. If you need to send manually, refer to hex (00 1E = 30s, for example).**

```js

   var method = msg.method;
var payload;

switch(method) {
    case 'set15s':  payload = 'AA8='; break;  // 15 s (00 0F), AA8= is b64
    case 'set30s':  payload = 'AB4='; break;  // 30 s (00 1E)
    case 'set1min': payload = 'ADw='; break;  // 1 min (00 3C)
    case 'set5min': payload = 'AHg='; break;  // 5 min (01 2C)
    case 'set10min':payload = 'Alg='; break;  // 10 min (02 58)
    case 'set15min':payload = 'A4Q='; break;  // 15 min (03 84)
    default:        payload = 'AB4=';
}
```

### Low Power Sleep
- Uses `ArduinoLowPower.sleep()` between transmissions
- Automatically disables Serial during sleep
- Wakes up at the configured interval

## Usage

1. **Install libraries** via Arduino Library Manager
2. **Update LoRaWAN credentials** (AppEUI and AppKey)
3. **Verify region setting** matches your location
4. **Upload code** to MKR WAN 1310
5. **Open Serial Monitor** (115200 baud) to see output
6. **Register device** in your LoRaWAN network (TTN, for example)

## Notes

- Setting up TTN with MKR-1310 (and getting keys):
https://docs.arduino.cc/tutorials/mkr-wan-1310/the-things-network/
- DS18B20:
https://www.digikey.com/en/htmldatasheets/production/1668/0/0/1/ds18b20z-t-r
