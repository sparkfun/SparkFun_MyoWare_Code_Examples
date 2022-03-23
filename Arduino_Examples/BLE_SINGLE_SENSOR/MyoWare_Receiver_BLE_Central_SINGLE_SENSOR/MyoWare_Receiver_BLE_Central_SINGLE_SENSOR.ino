/*
  MyoWare Receiver BLE Central SINGLE SENSOR Example
  SparkFun Electronics
  Pete Lewis
  3/17/2022

  This example sets up a SparkFun RedBoard Artemis as a BLE central device,
  Then, it connects to a second Artemis peripheral device that is reading a single MyoWare
  Muscle sensor. It then streams the data on the Serial Terminal.
  
  Note, in BLE, you have services, characteristics and values.
  Read more about it here:
  
  https://www.arduino.cc/reference/en/libraries/arduinoble/

  Note, before it begins checking the data and printing it,
  It first sets up some BLE stuff:
    1. sets up as a central
    2. scans for any peripherals
    3. Connects to the device named "MYOWARE1"
    4. Subscribes MYOWARE1's data characteristic

  In order for this example to work, you will need a second Artemis, and it will
  need to be programmed with the provided code specific to being a peripheral device, 
  and advertising as MYOWARE1 with the specific characteristic UUID.

  Note, both the service and the characteristic get unique UUIDs 
  (even though they are extremely close to being the same thing in this example)
  
  This Artemis, aka the "BLE Central," will subscribe to the peripheral board's 
  characteristic, and check to see if the value has been updated. When it has been 
  updated, it will print the value to the serial terminal.

  Hardware:
  SparkFun RedBoard Artemis
  USB from Artemis to Computer.
  
  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("MyoWare Single Sensor Example - BLE Central");

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
      byte received_val = 0;
      dataCharacteristic.readValue(received_val); // note, readValue returns nothing, and needs the value as a pointer/argument
      Serial.println(received_val);
    }
    delay(1);
  }
  Serial.println("Peripheral disconnected");
}
