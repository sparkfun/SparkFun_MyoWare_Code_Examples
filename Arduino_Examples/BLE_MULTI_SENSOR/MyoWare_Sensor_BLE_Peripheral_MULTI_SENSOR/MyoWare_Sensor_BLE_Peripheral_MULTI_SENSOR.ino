/*
  MyoWare Sensor BLE Peripheral MULTI SENSOR example
  SparkFun Electronics
  Pete Lewis
  3/23/2022

  This example reads 4 MyoWare Muscle Sensors, and then gets that data from this Artemis Redboard 
  (the Peripheral) to a second Redboard Artemis (the Central) over BLE.

  This Artemis, aka the "BLE Peripheral", will read the sensor on A0-A3.
  It will then store them in a single 32-bit variable, and then
  update that value to the "bluetooth bulliten board".

  Note, in BLE, you have services, characteristics and values.
  Read more about it here:
  
  https://www.arduino.cc/reference/en/libraries/arduinoble/

  Note, before it begins reading the ADC and updating the data,
  It first sets up some BLE stuff:
    1. sets up as a peripheral
    2. sets up a service and characteristic (the data)
        -Note, Services and characteristics have custom 128-bit UUID,
        -These must match the UUIDs in the code on the central device.
    3. advertises itself

  In order for this example to work, you will need a second Artemis, and it will
  need to be programmed with the provided code specific to being a central device, 
  looking for this specific peripheral/service/characteristic.

  Note, both the service and the characteristic get unique UUIDs 
  (even though they are extremely close to being the same thing in this example)
  
  The second Artemis, aka the "BLE Central", will subscribe to the first board's 
  charactieristic, and check to see if the value has been updated. When it has been 
  updated, it will read it, parse it into 4 separate bytes, then print the values 
  to the serial terminal.

  Hardware:
  4 MyoWare Sensors, each with a Link Shield snapped on top.
  TRS cables from Link shields to Analog ports (A0-A3) of Arduino Shield.
  Arduino Shield pressed into Artemis Redboard.
  USB from Artemis to Computer.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

BLEService sensorDataService("19b10000-e8f2-537e-4f6c-d104768a1214"); // BLE Service named "sensorDataService"

// BLE Data Characteristic - custom 128-bit UUID, readable, writable and subscribable by central
// Note, "BLENotify" is what makes it subscribable
BLEUnsignedLongCharacteristic dataCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite | BLENotify); 

const int ledPin = LED_BUILTIN; // pin to use for the LED

void setup() 
{
  Serial.begin(115200);
  // while (!Serial); // optionally wait for a serial terminal to open
  Serial.println("MyoWare Multi Sensor Example - BLE Peripheral");

  pinMode(ledPin, OUTPUT); // set LED pin to output mode

  if (!BLE.begin()) { // begin initialization
    Serial.println("starting BLE failed!");
    while (1);
  }
  Serial.println("BLE initiallized successfully");

  BLE.setLocalName("MYOWARE1"); // set advertised local name
  BLE.setAdvertisedService(sensorDataService); // set advertised service UUID
  sensorDataService.addCharacteristic(dataCharacteristic); // add the characteristic to the service
  BLE.addService(sensorDataService); // add service
  dataCharacteristic.writeValue(0); // set the initial value for the characeristic
  BLE.advertise(); // start advertising
}

void loop() 
{
  BLEDevice central = BLE.central(); // listen for BLE peripherals to connect

  if (central) // if a central is connected to peripheral
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address()); // print the central's MAC address

    Serial.println("Reading Sensor and writing BLE characteristic values now...");

    // while the central is still connected to peripheral:
    while (central.connected()) 
    {
      uint16_t val_A0 = analogRead(A0); // Read the sensor attached to Analog Pin A0
      uint16_t val_A1 = analogRead(A1); // Read the sensor attached to Analog Pin A1
      uint16_t val_A2 = analogRead(A2); // Read the sensor attached to Analog Pin A2
      uint16_t val_A3 = analogRead(A3); // Read the sensor attached to Analog Pin A3

      uint8_t val_A0_byte = map(val_A0, 0, 1023, 0, 255); // map the int to a byte
      uint8_t val_A1_byte = map(val_A1, 0, 1023, 0, 255); // map the int to a byte
      uint8_t val_A2_byte = map(val_A2, 0, 1023, 0, 255); // map the int to a byte
      uint8_t val_A3_byte = map(val_A3, 0, 1023, 0, 255); // map the int to a byte
      
      uint32_t output = 0; // output value (this will contain all 4 of our data bytes)

      // "OR" in all of our data bytes
      output |= val_A0_byte;            // ----  ----  ----  ----  ----  ----  XXXX  XXXX
      output |= (val_A1_byte << 8);     // ----  ----  ----  ----  XXXX  XXXX  ----  ----
      output |= (val_A2_byte << 16);    // ----  ----  XXXX  XXXX  ----  ----  ----  ----
      output |= (val_A3_byte << 24);    // XXXX  XXXX  ----  ----  ----  ----  ----  ----
      
      delay(10);
      dataCharacteristic.writeValue(output); // "post" to "BLE bulletin board"
      // Note, because our second Artemis in this example (the central) is subscribed to this characteristic,
      // it can simply call Characteristic.valueUpdated() to see if it has been updated.
      // valueUpdated() will return True if updated, or false if no update has happened.
      // If it has been updated, the central Artemis can read the latest value using Characteristic.readValue();
    }
    Serial.print(F("Disconnected from central: ")); // when the central disconnects, print it out
    Serial.println(central.address());
  }
}
