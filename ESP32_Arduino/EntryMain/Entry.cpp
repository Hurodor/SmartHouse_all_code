#include "Entry.h"

// Attaches door servo to correct pin.
void Entry::init(){
  doorServo.attach(servoPin, 500, 2400);
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  lockDoor();
}

// Opens door if its closed and not locked.
void Entry::openDoor(){
  if (!doorOpen && !locked){
    for (int i = 0; i < 100; ++i){
      doorServo.write(i);
      delay(1);
    }
    Serial.println("DOOR OPENED");
    doorOpen = true;
  }
  updateLeds();
}

// Closes door if its open.
void Entry::closeDoor(){
  if (doorOpen){
    for (int i = 100; i > 0; --i){
      doorServo.write(i);
      delay(1);
    }
    Serial.println("DOOR CLOSED");
  }
  doorOpen = false;
  updateLeds();
}

void Entry::addPerson(){
  ++people;
  Serial.println("PERSON ENTERED");
  if (people > maxPeople){
    Serial.println("TOO MANY PEOPLE INSIDE");
    tone(piezoPin, 440);
  }
}

void Entry::removePerson(){
  if (people > 0){
    Serial.println("PERSON LEFT");
    --people;
  }
  if (people <= maxPeople){
    noTone(piezoPin);
  }
}

void Entry::lockDoor(){
  if (!doorOpen){
    locked = true;
    Serial.println("DOOR LOCKED");
  }
  else {
    closeDoor();
    lockDoor();
  }
  updateLeds();
}

void Entry::unlockDoor(){
  locked = false;
  Serial.println("DOOR UNLOCKED");
  updateLeds();
}

void Entry::updateLeds(){
  if (locked){
    digitalWrite(redLed, HIGH);
    digitalWrite(yellowLed, LOW);
    digitalWrite(greenLed, LOW);
  }
  else if (!locked && doorOpen){
    digitalWrite(redLed, LOW);
    digitalWrite(yellowLed, LOW);
    digitalWrite(greenLed, HIGH);
  }
  else if (!locked && !doorOpen){
    digitalWrite(redLed, LOW);
    digitalWrite(yellowLed, HIGH);
    digitalWrite(greenLed, LOW);
  }
}

String Entry::signalString(){
  // Signal string structure - {"people": 00, "doorOpen": 0, "doorLocked": 0}
  String p = "\"people\": " + String(people);
  String dO = ", \"doorOpen\": " + String(doorOpen);
  String dL = ", \"doorLocked\": " + String(locked);
  String sig = "{" + p + dO + dL + "}";
  return sig;
}

void Entry::interpretSignal(String sig){
  // Incoming signal string shape = "0000"

  bool doorState;
  bool doorLock;
  int newMaxPeople;
  
  doorState = sig[1] - '0';
  doorLock = sig[2] - '0';
  if (doorLock) {lockDoor();} else {unlockDoor();}
  if (doorState) {openDoor();} else {closeDoor();}
  newMaxPeople = 10 * (sig[3] - '0');
  newMaxPeople += (sig[4] - '0');
  setMaxPeople(newMaxPeople);
}
