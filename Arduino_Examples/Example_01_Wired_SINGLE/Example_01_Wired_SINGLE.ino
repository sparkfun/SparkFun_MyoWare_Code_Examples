/*
  MyoWare Example 01 Wired - single sensor
  SparkFun Electronics
  Pete Lewis
  3/24/2022
  License: This code is public domain but you buy me a beverage if you use this and we meet someday.

  This code was adapted from the MyoWare analogReadValue.ino example found here:
  https://github.com/AdvancerTechnologies/MyoWare_MuscleSensor

  This example streams the data from a single MyoWare sensor attached to ADC A0.

  *Only run on a laptop using its battery. Do not plug in laptop charger/dock/monitor.
  
  *Do not touch your laptop trackpad or keyboard while the MyoWare sensor is powered.

  Hardware:
  SparkFun RedBoard Artemis (or Arduino of choice)
  USB from Artemis to Computer.
  Output from sensor connected to your Arduino pin A0
  
  This example code is in the public domain.
*/

void setup() 
{
  Serial.begin(115200);
  while (!Serial); // optionally wait for serial terminal to open
  Serial.println("MyoWare Single Sensor Example 01");
}

void loop() 
{  
  int sensorValue = analogRead(A0); // read the input on analog pin A0

  Serial.println(sensorValue); // print out the value you read

  delay(50); // to avoid overloading the serial terminal
}