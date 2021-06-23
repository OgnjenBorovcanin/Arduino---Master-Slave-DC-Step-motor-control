#include <Wire.h>
//#include <SPI.h>
#include <Servo.h>

const int cs = 5;

#define STEPPER_PIN_1 9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12
int step_number = 0;
boolean changeDirection = false;
int servoPin = 6;
int potencPin = 0;

Servo servo1;

boolean potencUsed = false;

boolean configurationChosen = false;
boolean protocolChosen = false;
boolean I2C = true; //if it's set false, SPI is selected

String serialString;  //serialStringRead();
String protocol;

int machineState = 0; //0 - idle, 1 - servo, 2 - step


void setup() {

  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);

  Serial.begin(9600);
  //CONFIGURATION
  if (!configurationChosen) {
    Serial.println("1 - Default configuration");
    Serial.println("2 - Custom configuration");
    setupStringRead();
    if (serialString == "1\n") {
      configurationChosen = true;
      serialString = "";
      Serial.println("Default configuration selected.");
      Serial.println("Servo motor - pin 9");
      servo1.attach(servoPin);
    } else if (serialString == "2\n") {
      configurationChosen = true;
      serialString = "";
      Serial.println("Custom configuration selected.");
      //custom pins
    } else {
      Serial.println("Unknown command.");
      delay(20);
      configurationChosen = false;
      serialString = "";
      setup();
    }
  }

  //PROTOCOL
  if (!protocolChosen) {
    Serial.println("Choose protocol (I2C or SPI):");
    setupStringRead();
    if (serialString == "I2C\n") {
      serialString = "";
      protocolChosen = true;
      Serial.println("I2C protocol selected.");
      I2C = true;
      Wire.begin();
    } else if (serialString == "SPI\n") {
      serialString = "";
      protocolChosen = true;
      Serial.println("SPI protocol selected.");
      I2C = false;
      pinMode(cs, OUTPUT);
      //SPI.begin ();
      //SPI.setClockDivider(SPI_CLOCK_DIV8);
     // SPI.setBitORder(MSBFIRT);
    } else {
      Serial.println("Unknown command");
      delay(20);
      serialString = "";
      protocolChosen = false;
      setup();
    }
  }
}

byte x = 0;
int servoPosition;
int stepSpeed = 2;

void loop() {

  switch (machineState) {
    case 0: //idle
      //Serial.println("Idle state");
      if (Serial.available()) {
        String tmpString = Serial.readString();
        checkStateChange(tmpString);
      }
      break;
    case 1: //servo -> (0-180) SERVO MODE
      //Serial.println("Servo state");
      if (Serial.available()) {
        String tmpString = Serial.readString();
        checkStateChange(tmpString);  //check state change 1st
        if (machineState != 1) {
          serialString = "";
          break;
        }
        if (tmpString == "potenc\n") { //check if potenc is used 2nd
          potencUsed = !potencUsed; //revert boolean for on/off
        }
        servoPosition = tmpString.toInt();  //if not, it's from serial
      }

      if (potencUsed) { //if it feeds via potentiometer
        servoPosition = analogRead(potencPin) / 5; //BAGUJE GLUPI POTENCIOMETAR
        Serial.println(servoPosition);
      }
      if (servoPosition > 180) {
        servoPosition = 180;
      } else if (servoPosition < 0) {
        servoPosition = 0;
      }
      servo1.write(servoPosition);

//      //send to slave
      if (I2C) {  //ako je I2C
        Wire.beginTransmission(10); // transmit to device #8
        Wire.write("Servo position is: "); // sends five bytes
        Wire.write(servoPosition); // sends one byte
        Wire.endTransmission(); // stop transmitting
      } else {  //ako je SPI
        // enable Slave Select
        digitalWrite(SS, LOW);    // SS is pin 10
        // send servo position
//        for (const char * p = "Hello, world!\n" ; c = *p; p++)
//          SPI.transfer (c);

        // disable Slave Select
        digitalWrite(SS, HIGH);
      }

      break;
    case 2: //STEP MODE
      //Serial.println("Step state");
      if (Serial.available()) {
        String tmpString = Serial.readString();
        checkStateChange(tmpString);  //check state change
        if (machineState != 2) {
          serialString = "";
          break;
        }
        if (tmpString == "potenc\n") { //check if potenc is used 2nd
          potencUsed = !potencUsed; //revert boolean for on/off
        }
        if (tmpString == "change direction\n") { //if to change direction
          changeDirection = !changeDirection; //revert boolean for changing direction
        } else {
          stepSpeed = tmpString.toInt();  //if not, it's from serial
        }
      }

      if (potencUsed) { //if it feeds via potentiometer
        stepSpeed = (analogRead(potencPin) / 6) - 30; //BAGUJE GLUPI POTENCIOMETAR
        Serial.println(stepSpeed);
      }
      if (stepSpeed > 500) {
        stepSpeed = 500;
      } else if (stepSpeed < 2) {
        stepSpeed = 2;
      }
      OneStep(changeDirection);
      delay(stepSpeed);
      Serial.println(stepSpeed);

      //send to slave
      if (I2C) {  //ako je I2C
        Wire.beginTransmission(10); // transmit to device #8
        Wire.write("Step speed is: "); // sends five bytes
        Wire.write(stepSpeed); // sends one byte
        Wire.endTransmission(); // stop transmitting
      } else {  //ako je SPI

      }

      break;
    default:  //idle
      //Serial.println("Idle state");
      if (Serial.available()) {
        String tmpString = Serial.readString();
        checkStateChange(tmpString);
      }
      break;
  }

  //  if (I2C) {  //ako je I2C
  //    Wire.beginTransmission(10); // transmit to device #8
  //    // Wire.write("x is "); // sends five bytes
  //    Wire.write(x); // sends one byte
  //    Wire.endTransmission(); // stop transmitting
  //
  //    x++;
  //    delay(500);
  //  } else {  //ako je SPI
  //
  //  }
}

void setupStringRead() {
  while (!Serial.available()) {
    ;
  }
  while (Serial.available()) {
    serialString = Serial.readString(); // read the incoming data as string
  }
}

void serialStringRead() {
  while (Serial.available()) {
    serialString = Serial.readString(); // read the incoming data as string
  }
}

void checkStateChange(String myString) {
  if (myString == "idle\n") {
    Serial.println("Idle state");
    machineState = 0;
  } else if (myString == "servo\n") {
    Serial.println("Servo state");
    machineState = 1;
  } else if (myString == "step\n") {
    Serial.println("Step state");
    machineState = 2;
  } else {

  }
}

void OneStep(bool dir) {
  if (dir) {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
    }
  } else {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);


    }
  }
  step_number++;
  if (step_number > 3) {
    step_number = 0;
  }
}
