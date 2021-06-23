#include <Wire.h>
#include <SPI.h>

boolean protocolChosen = false;
String serialString;
boolean I2C = true; //if it's set false, SPI is selected

void setup() {
  Serial.begin(9600);

  //PROTOCOL
  if (!protocolChosen) {
    Serial.println("Choose protocol (I2C or SPI):");
    serialStringRead();
    if (serialString == "I2C\n") {
      serialString = "";
      protocolChosen = true;
      Serial.println("I2C protocol selected.");
      I2C = true;
      Wire.begin(10); // join i2c bus with address #10
      Wire.onReceive(receiveEvent); // register event
    } else if (serialString == "SPI\n") {
      serialString = "";
      protocolChosen = true;
      Serial.println("SPI protocol selected.");
      I2C = false;
      //OTVARANJE SPI KONEKCIJE
    } else {
      Serial.println("Unknown command");
      delay(20);
      serialString = "";
      protocolChosen = false;
      setup();
    }
  }
  
}

void loop() {
}

void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c); // print the character
  }
  int x = Wire.read(); // receive byte as an integer
  Serial.println(x); // print the integer
}

void serialStringRead() {
  while (!Serial.available()) {
    ;
  }
  while (Serial.available()) {
    serialString = Serial.readString(); // read the incoming data as string
  }
}
