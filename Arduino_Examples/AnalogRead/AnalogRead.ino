/*
  AnalogRead.ino
  Reads an analog input on pin 0, converts it to voltage, and prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).

  This example code is in the public domain.
*/

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {

  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);

  // print out the value you read:
  Serial.println(sensorValue);
  
  // Convert the analog reading (which goes from 0 - 1023 for 10-bit ADC) to a voltage (0 - 5.0V, for 5V Arduino):
  //float voltage = sensorValue * (5.0 / 1023.0);
  // Convert the analog reading (which goes from 0 - 1023 for 14-bit ADC) to a voltage (0 - 3.3V, for 3.3V Arduino):
  //float voltage = sensorValue * (3.3 / 16384.0);

  // print out the value you read:
  //Serial.println(voltage);

}
