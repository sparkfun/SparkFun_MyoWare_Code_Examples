/*
  MyoWare Receiver BLE Central Muscle Drums
  SparkFun Electronics
  Rob Reynolds and Pete Lewis
  Original creation date: March 16, 2022
  License: This code is public domain but you buy me a beverage if you use this and we meet someday.

  This example allows you to make sounds using your muscles as triggers!

  It requires the following hardware:
  2 SparkFun Artemis Redboards (https://www.sparkfun.com/products/15444)
  1 Tsunami WAV Trigger (Qwiic) (https://www.sparkfun.com/products/18159)
  MyoWare Arduino Shield, Sensors/link-shields, TRS cables (https://www.sparkfun.com/products/18977)
  Battery pack (for the Artemis/sensor on your body) (https://www.sparkfun.com/products/9835)

  One Artemis is on your body (or near you) and listens to 4 muscle sensors.
  
  Another Artemis is "listening" to sensor data via BLE and commanding the Tsunami to play sounds.
  
  We set up this Artemis Redboard as a BLE Central device,
  Then, it connects to a second Artemis Peripheral Device that is reading multiple MyoWare
  Muscle sensors. As you flex your muscles, the data is sent via BLE, and the Central
  Artemis looks at the values and plays sounds when you flex.
  
  Note, this "central artemis" also streams the data on the Serial Terminal.
  This is useful for watching the data output from each sensor in real time,
  And adjusting your thresholds as needed.

  Note, in BLE, you have services, characteristics and values.
  Read more about it here:

  https://www.arduino.cc/reference/en/libraries/arduinoble/

  Note, before it begins checking the data and printing it,
  It first sets up some BLE stuff:
    1. sets up as a central
    2. scans for any peripherals
    3. Connects to the device named "MAYOWARE1"
    4. Subscribes MYOWARE1's data characteristic

  In order for this example to work, you will need your peripheral Artemis
  to be programmed with the provided code specific to being a peripheral device,
  and advertizing as MYOWARE1 with the specific characteristic UUID. This peripheral
  sketch is in this github repo and named, "MyoWare_Sensor_BLE_Peripheral_MULTI_SENSOR.ino"
  It is located here:
  https://github.com/sparkfun/SparkFun_MyoWare_Arduino_Examples/tree/main/Arduino_Examples/BLE_MULTI_SENSOR/MyoWare_Sensor_BLE_Peripheral_MULTI_SENSOR

  Note, both the service and the characteristic get unique UUIDs
  (even though they are extremely close to being the same thing in this example)

  This Artemis, aka the "BLE Central", will subscribe to the peripheral board's
  charactieristic, and check to see if the value has been updated. When it has been
  updated, it will read the new values and play sounds.

  Note, the value that this Central Device will read is acutally a uint32_t
  (aka "unsigned long" in arduino). This single variable will carry all of the
  sensor values, stored as 4 bytes within it.

  Hardware Hookup:
  USB from Artemis to Computer.
  Qwiic cable from Artemis to Tsunami
  Tsunami "Output 0" (sig & GND) wired to audio amp/speaker of choice.

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

#include <SparkFun_Tsunami_Qwiic.h> //http://librarymanager/All#SparkFun_Tsunami_Super_WAV_Trigger
TsunamiQwiic tsunami;

// Trigger Thresholds
// Global variables to store each sensor thresholds as bytes (0-255).
// These will be used to trigger sounds when ADC values go above their threshold.
// Note, 100 is a pretty good starting point, but you may need to adjust these.
uint8_t A0_thresh = 100;
uint8_t A1_thresh = 100;
uint8_t A2_thresh = 100;
uint8_t A3_thresh = 100;

// "armed" booleans
// These are used to know if the current muscle is armed for another play.
// This requires that the muslce be relaxed before triggering another sound.
// It prevents the same sound being triggered repeatedly too quickly.
boolean A0_armed = false;
boolean A1_armed = false;
boolean A2_armed = false;
boolean A3_armed = false;

// WAVE File number defines
// (these match up with the actual WAV file names on the uSD card)
// These example files for drum sounds can be found here:
// https://github.com/sparkfun/SparkFun_Tsunami_Super_WAV_Trigger_Qwiic/tree/main/Example_WAV_Files/MONO
#define KICK 101
#define SNARE 102
#define HAT 103
#define RIDE 104

void setup()
{
  Serial.begin(115200);
  //while (!Serial); // optionally wait for serial terminal to open
  Serial.println("MyoWare Muscle Drums Example - BLE Central");

  Wire.begin();

  // Check to see if Tsunami Qwiic is present on the bus
  // Note, here we are calling begin() with no arguments = defaults (address:0x13, I2C-port:Wire)
  if (tsunami.begin() == false)
  {
    Serial.println("Tsunami Qwiic failed to respond. Please check wiring and possibly the I2C address. Freezing...");
    while (1);
  };
  Serial.println("Tsunami initiallized successfully");

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

      play_sounds();
    }
    delay(1);
  }
  Serial.println("Peripheral disconnected");
}

// play_sounds
// look at global variables from sensors (that we update via BLE)
// check against thresholds, then if flexed, play sounds
//
// Note, we also have an "armed" feature. This prevents unwanted
// super-fast repeated triggering. It reuires that the signal drop back down
// below the threshold in order to be "armed" again and play a sound.
// *much like it only triggers on a "rising edge" of sensor data.
void play_sounds()
{
  if ((val_A0_byte > A0_thresh) && A0_armed)
  {
    tsunami.trackPlayPoly(KICK, 0); // track 1, output 0
    A0_armed = false;
  }
  else if (val_A0_byte < A0_thresh){
    A0_armed = true;
  }
  if ((val_A1_byte > A1_thresh) && A1_armed)
  {
    tsunami.trackPlayPoly(SNARE, 0);
    A1_armed = false;
  }  else if (val_A1_byte < A1_thresh){
    A1_armed = true;
  }
  if ((val_A2_byte > A2_thresh) && A2_armed)
  {
    tsunami.trackPlayPoly(HAT, 0);
    A2_armed = false;
  }
  else if (val_A2_byte < A2_thresh){
    A2_armed = true;
  }
  if ((val_A3_byte > A3_thresh) && A3_armed)
  {
    tsunami.trackPlayPoly(RIDE, 0);
    A3_armed = false;
  }
    else if (val_A3_byte < A3_thresh){
    A3_armed = true;
  }
}
