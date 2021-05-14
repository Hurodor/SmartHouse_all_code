#include "Entry.h"
#include "Request.h"

// --------------------------------------------------------------------------------//
  
const char* id = "Hurodor";
const char* pw = "123456789";
const char* baseURL= "http://172.20.10.2:5000/";

// --------------------------------------------------------------------------------//
// Pins.

const int doorPin = 34;
const int lockPin = 35;
const int servoPin = 32;
const int redLed = 25;
const int yellowLed = 26;
const int greenLed = 27;
const int enteringInput = 18;
const int leavingInput = 19;
const int codeInput = 14;
const int piezoPin = 33;
const int doorbell = 13;

// --------------------------------------------------------------------------------//
// Boolean sensor values.

bool doorPressed;
bool lockPressed;
bool personEntered;
bool personLeft;

// --------------------------------------------------------------------------------//
// Timer variables.

unsigned long timer;
unsigned long getTimer = millis();
const int getInterval = 5000;

// --------------------------------------------------------------------------------//
// Signal variables.

String inputSignal;
String oldInputSignal;
String outputSignal;
String oldOutputSignal = "";

// --------------------------------------------------------------------------------//
// Name/ID of room. Used for endpoint in signal transfer.

String room = "entry";

// --------------------------------------------------------------------------------//

Servo servo;
Entry entry{servo, 5, servoPin, piezoPin, redLed, yellowLed, greenLed};
Request r(id, pw, baseURL);

void setup(){
  pinMode(doorPin, INPUT);
  pinMode(lockPin, INPUT);
  pinMode(enteringInput, INPUT);
  pinMode(leavingInput, INPUT);
  pinMode(doorbell, INPUT);
  
  Serial.begin(115200);
  
  entry.init();
  r.init();
  timer = millis();
}

void loop(){

  // --------------------------------------------------------------------------------//
  // Receive and send signal.

  // If we want to limit the amount of get-requests sent to the server, we uncomment line 79 & 90.
  //if (millis() - getTimer > getInterval){
   inputSignal = r.get(room);
   getTimer = millis();
   if (inputSignal != oldInputSignal){
     if (inputSignal == "") {inputSignal = oldInputSignal;}
     entry.interpretSignal(inputSignal);
     oldInputSignal = inputSignal;
    }
  //}
  
  outputSignal = entry.signalString();
  if (outputSignal != oldOutputSignal){
    r.post(room, outputSignal);
    oldOutputSignal = outputSignal;
  }

  // --------------------------------------------------------------------------------//
  // Get button presses.
  
  doorPressed = digitalRead(doorPin);
  lockPressed = digitalRead(lockPin);

  // --------------------------------------------------------------------------------//
  // Doorbell function - does not work when there are too many people in the house.

 if (digitalRead(doorbell) == 1 && (entry.getPeople() <= entry.getMaxPeople())){
   while(digitalRead(doorbell) == 1){
     tone(33, 880);
   }
   noTone(33);
 }

  // --------------------------------------------------------------------------------//
  // Checks for correct code input.

  if (digitalRead(codeInput) == 1){
    Serial.println("Code correct.");
    if (entry.getLocked() == 1){
      entry.unlockDoor();
    }
    entry.openDoor();
    delay(10);
    while(digitalRead(codeInput)){}
  }

  // --------------------------------------------------------------------------------//
  // Locks and unlocks the door.
  
  if (lockPressed == 1){
    
    if (entry.getLocked() == true){
      entry.unlockDoor();
    }
    else {
      entry.lockDoor();
    }
    
    while (lockPressed == 1){
      lockPressed = digitalRead(lockPin);
      delay(25);
    }
  }

  // --------------------------------------------------------------------------------//
  // Opens and closes the door.
  
  if (doorPressed == 1){
    
    if (entry.getDoorOpen() == true){
      entry.closeDoor();
    }
    else {
      entry.openDoor();
      timer = millis();
      if (entry.getLocked() == false){
      }
    }
    
    while(doorPressed == 1){
      doorPressed = digitalRead(doorPin);
      delay(25);
    }
  }

  // --------------------------------------------------------------------------------//
  // Checks for people entering or leaving the house.
  
  if (entry.getDoorOpen() == true){
    personEntered = digitalRead(enteringInput);
    personLeft = digitalRead(leavingInput);

    // The direction sensor circuit is connected to the room ESP32 via the ESP32's 5V & GND
    // to the Arduino's Vin and GND. This voltage is lower than the recommended for Vin on
    // the Arduino. Because of this the door servo moving causes inconsistencies in the voltage
    // and thus we need a timer to make sure the ESP32 does not read these noise signals as
    // actual signals. (Alternative fix is to externally power the Arduino.)
    
    if (millis() - timer > 2000){
      if (personEntered){
        entry.addPerson();
        personEntered = 0;
        delay(500);
      }
      else if (personLeft){
        entry.removePerson();
        personLeft = 0;
        delay(500);
      }
    }
  }
}
