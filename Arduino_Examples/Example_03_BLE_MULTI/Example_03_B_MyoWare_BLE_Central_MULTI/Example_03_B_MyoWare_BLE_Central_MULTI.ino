/*
  MyoWare Receiver BLE Central MULTI SENSOR example
  SparkFun Electronics
  Pete Lewis
  3/23/2022

  This example sets up a SparkFun Artemis Redboard as a BLE Central device,
  Then, it connects to a second Artemis Peripheral Device that is reading multiple MyoWare
  Muscle sensors. It then streams the data on the Serial Terminal.

  Note, in BLE, you have services, characteristics and values.
  Read more about it here:

  https://www.arduino.cc/reference/en/libraries/arduinoble/

  Note, before it begins checking the data and printing it,
  It first sets up some BLE stuff:
    1. sets up as a central
    2. scans for any peripherals
    3. Connects to the device named "MAYOWARE1"
    4. Subscribes MYOWARE1's data characteristic

  In order for this example to work, you will need a second Artemis, and it will
  need to be programmed with the provided code specific to being a peripheral device,
  and advertizing as MYOWARE1 with the specific characteristic UUID.

  Note, both the service and the characteristic get unique UUIDs
  (even though they are extremely close to being the same thing in this example)

  This Artemis, aka the "BLE Central", will subscribe to the peripheral board's
  charactieristic, and check to see if the value has been updated. When it has been
  updated, it will print the value to the serial terminal.

  Note, the value that this CEntral Device will read is acutally a uint32_t 
  (aka "unsigned long" in arduino). This single variable will carry all of the 
  sensor values, stored as 4 bytes within it.

  Hardware:
  SparkFun Artemis Redboard
  USB from Artemis to Computer.

  ** For consistent BT connection follow these steps:
  ** 1. Reset Peripheral
  ** 2. Wait 5 seconds
  ** 3. Reset Central
  ** 4. Enjoy BT connection
  **
  ** ArduinoBLE does not support RE-connecting two devices.
  ** If you loose connection, you must follow this hardware reset sequence again.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

// global variables to store each sensor values as bytes (0-255)
uint8_t val_A0_byte = 0;
uint8_t val_A1_byte = 0;
uint8_t val_A2_byte = 0;
uint8_t val_A3_byte = 0;

void setup()
{
  Serial.begin(115200);
  // while (!Serial); // optionally wait for serial terminal to open
  Serial.println("MyoWare Multi Sensor Example - BLE Central");

  if (!BLE.begin()) // initialize the BLE hardware
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
  Serial.println("BLE initiallized successfully");

  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214"); // start scanning for peripherals
}

void loop()
{
  BLEDevice peripheral = BLE.available(); // check if a peripheral has been discovered

  if (peripheral) // discovered a peripheral, print out its info
  {
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "MYOWARE1")
    {
      return;
    }

    BLE.stopScan();

    checkUpdate(peripheral);

    Serial.println("Starting to scan for new peripherals again...");
    BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214"); // peripheral disconnected, scan again
    Serial.println("Scan has begun...");
  }
}

// Connect to peripheral
// Then continue to check if the data has been updated,
// If so, print it to terminal
void checkUpdate(BLEDevice peripheral)
{
  Serial.println("Connecting ..."); // connect to the peripheral

  if (peripheral.connect())
  {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Discovering attributes ..."); // discover peripheral attributes
  if (peripheral.discoverAttributes())
  {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the data characteristic
  BLECharacteristic dataCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!dataCharacteristic)
  {
    Serial.println("Peripheral does not have that characteristic!");
    peripheral.disconnect();
    return;
  } else if (!dataCharacteristic.canWrite())
  {
    Serial.println("Peripheral does not have a writable characteristic!");
    peripheral.disconnect();
    return;
  } else if (!dataCharacteristic.canRead())
  {
    Serial.println("Peripheral does not have a readable characteristic!");
    peripheral.disconnect();
    return;
  } else if (!dataCharacteristic.canSubscribe())
  {
    Serial.println("Characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!dataCharacteristic.subscribe())
  {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) // while the peripheral is connected
  {
    if (dataCharacteristic.valueUpdated()) // Check to see if the value of the characteristic has been updated
    {
      uint32_t received_val = 0;
      dataCharacteristic.readValue(received_val); // note, "readValue(uint32_t& value)" needs the variable to be passed by reference

      // parse received_val - this contains all 4 of our ADC values (as each byte)
      val_A0_byte = (received_val & 0x000000FF);
      val_A1_byte = ((received_val & 0x0000FF00) >> 8);
      val_A2_byte = ((received_val & 0x00FF0000) >> 16);
      val_A3_byte = ((received_val & 0xFF000000) >> 24);

      //Serial.print(received_val, HEX); // optional print of the entire uint32_t for debugging
      //Serial.print("\t");
      Serial.print(val_A0_byte);
      Serial.print("\t");
      Serial.print(val_A1_byte);
      Serial.print("\t");
      Serial.print(val_A2_byte);
      Serial.print("\t");
      Serial.println(val_A3_byte);
    }
    delay(1);
  }
  Serial.println("Peripheral disconnected");
}
