#include <Arduino.h>
#include <ESP32Servo.h>
#include <analogWrite.h>

#include <ESP32Tone.h>
#include <ESP32PWM.h>

#ifndef Room_h
#define Room_h

enum class RoomNumber{KITCHEN = 7, BATHROOM = 8, LIVINGROOM = 9};



class Room{
protected:
    RoomNumber roomID;

    int lightValue;
    int temperatureVal;
    int winAngle;
    int numPeople;
    int maxPeople;
    int fanValue;
    String& userID;

    // Special formattet string for local booking
    String bookingString;
    String startHour;
    String endHour;
    int date;
  
    // Objects every room needs.
    Servo& windowServo; 
    
    // PWM properties DC-MOTORs
    int enablePin;

    // PWM Properties LED-controller
    int ledPin;
    int ledPinOven;

    // Servo
    int windowServoPin;

    // PWM Buzzer
    int buzzerGPIOpin;

public:
    // Constructor. Requires Servo object and user ID
    Room(Servo& windowServo, String& userID);

    // Init member-functions
    void init();

    // Input hour and minutes in string-format and convert to one string
    void setStartHour( const String& hh, const String& mm) {startHour = hh + ":" + mm + ":00";}
    void setEndHour(const String& hh, const String& mm) {endHour = hh + ":" + mm + ":00";}
    String getStartHour() const {return startHour;}
    String getEndHour() const {return endHour;}

    // Sets the booking-string to correct format
    void bookResource(int roomNumber, int date, String startHour, String endHour);
    
    // Returns the booking-string
    String getBookingString() const {return bookingString;}

    void showBooking(String endpoint);

    // Change status on front door
    String setDoorStatus(bool doorOpen, bool locked);

    // Functions related to people in the room
    int getPeople() const {return numPeople;}
    void setPeople(const int& n);
    void incrementPeople();
    void decrementPeople();
    void setMaxPeople(int newMax){maxPeople = newMax;}
    int getMaxPeople(){return maxPeople;}

    // Functions related to the fan
    void setFanValue(const uint8_t& value);
    void setFanValueMode(const uint8_t& mode);
    int getFanValue() const {return fanValue;}

    // Functions related to the temperature
    void setTemperature(const uint8_t& temp);
    int getTemperature() const {return temperatureVal;}
    
    // Functions related to the lightning
    void setLightValue(const uint8_t& value);
    int getLightValue() const {return lightValue;}
    
    // Functions related to the window
    void setWindowAngleManual(const int& angle);
    void setWindowAngleAuto(const int& mode);
    int getWindowAngle() const {return winAngle;}
    
    // Functions related to communication
    String makeSignal();
    String sendLight(int lightValue, int idRoom);
    String sendTemp(int temperatureVal, int idRoom);
    String sendAngle(int windowAngle, int idRoom);
    String sendFan(int fanValue, int idRoom);
    String makeEntrySignal(int openDoor, int lockDoor);
    
    // Split signals by index
    // Subtract char value '0' so that translation from char to int is correct
    // Multiply by factor of 100 and 10 to make the value correct
    void interpretSignal(String sig);
    
};


#endif
