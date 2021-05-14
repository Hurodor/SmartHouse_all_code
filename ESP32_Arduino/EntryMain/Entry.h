#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

#ifndef Entry_h
#define Entry_h

class Entry{
  private:
    Servo& doorServo;
    int servoPin;
    int piezoPin;
    int maxPeople;
    int people;
    bool locked;
    bool doorOpen;
    int redLed;
    int yellowLed;
    int greenLed;

  public:
  // Class constructor sets all private member variables to appropriate values.
    Entry(Servo& ds, int mp, int sp, int pp, int rlp, int ylp, int glp): 
      doorServo{ds}, 
      servoPin{sp},
      piezoPin{pp}, 
      maxPeople{mp}, 
      people{0}, 
      locked{false},
      doorOpen{false},
      redLed{rlp},
      yellowLed{ylp},
      greenLed{glp}
      {}

    // Initializes all parts of the class that needs to be configured in setup. init() should always be
    // used inside the void setup function of the .ino file.
    void init();

    // All functions related to the door.
    void openDoor();
    void closeDoor();
    bool getDoorOpen() const { return doorOpen; }
    void lockDoor();
    void unlockDoor();
    bool getLocked() { return locked; }

    // All functions related to people.
    void setMaxPeople(int newMax) { maxPeople = newMax; }
    int getMaxPeople() const { return maxPeople; }
    void addPerson();
    void removePerson();
    int getPeople() { return people; }

    // Reads private member variables and updates LEDs accordingly.
    void updateLeds();

    // All functions related to creating and interpreting signals.
    String signalString();
    void interpretSignal(String sig);
};

#endif
