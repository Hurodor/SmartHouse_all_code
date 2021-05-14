#include "Room.h"

Room::Room(Servo& ws, String& userID)
    :windowServo{ws}, userID{userID}{
    
    // PWM properties DC-MOTOR
    enablePin = 14;

    // PWM properties LED-controller
    ledPin = 5;
    ledPinOven = 18;

    // Servo properties
    windowServoPin = 16;

    buzzerGPIOpin = 27;
    startHour = "HH:MM:SS";
    endHour = "HH:MM:SS";

}
void Room::bookResource(int roomNumber, int date, String startHour, String endHour){
    // Litt usikker på hva jeg tenkte her - Petter
    roomID = static_cast<RoomNumber>(roomNumber);
    bookingString = "{\"bookFrom\": \"" + startHour + '\"' + ", " + "\"bookTo\": "  + "\"" + endHour + "\", " + "\"date\": "  + "\"" + String(date) + "\", " + "\"userID\": " + "\"" + userID + "\", " + "\"roomID\": " + "\"" + String(static_cast<int>(roomID)) + "\"" + "}";
}
void Room::init(){
    pinMode(enablePin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(ledPinOven, OUTPUT);
    pinMode(buzzerGPIOpin, OUTPUT);
    windowServo.attach(windowServoPin, 500, 2400);
}

void Room::setPeople(const int& n){
    if (n >= 0 && n <= maxPeople){
        numPeople = n;
    }
}
void Room::incrementPeople(){
    ++numPeople;
    if (numPeople > maxPeople){
        tone(buzzerGPIOpin, 440);
    }
}
void Room::decrementPeople(){
    if (numPeople > 0){
        --numPeople;
    }
    if (numPeople <= maxPeople){
        noTone(buzzerGPIOpin);
    }
}
void Room::setFanValue(const uint8_t& value){
    bool fromOffState = (fanValue < 50);
    if (value < 0 ) {fanValue = 0;}
    else if (value > 255) {fanValue = 255;}
    else {fanValue = value;}
    if (fromOffState){analogWrite(enablePin, 100); delay(10);}
    analogWrite(enablePin, fanValue);
}

void Room::setTemperature(const uint8_t& temp){
    if (temp < 15) {temperatureVal = 15;}
    else if (temp > 25) {temperatureVal = 25;}
    else {temperatureVal = temp;}
    int heaterLightVal = map(temperatureVal, 15, 25, 0, 255);
    analogWrite(ledPinOven, heaterLightVal);
}
void Room::setLightValue(const uint8_t& value){
    if (value < 0) {lightValue = 0;}
    else if (value > 255) {lightValue = 255;}
    else {lightValue = value;}
    analogWrite(ledPin, lightValue);
}

void Room::setWindowAngleManual(const int& angle){
    if (angle < 0) {winAngle = 0;}
    else if (angle > 90) {winAngle = 90;}
    else {winAngle = angle;}
    windowServo.write(winAngle);
}

// doorOpen: open true, closed false; locked: 1 true, 0 false
String Room::setDoorStatus(bool doorOpen, bool locked){
    String sig = "{\"doorOpen\": " + String(doorOpen) + ", \"doorLocked\": " + String(locked) + "}";
    return sig;
}

String Room::makeSignal(){
  // {"lightVal": 000, "tempVal": 00, "windowOpen": 000, "fanSpeed": 000}
  String lv = "\"lightVal\": " + String(lightValue);
  String tv = ", \"tempVal\": " + String(temperatureVal);
  String wo = ", \"windowOpen\": " + String(winAngle);
  String fs = ", \"fanSpeed\": " + String(fanValue);
  String id = ", \"roomID\": " + String(userID);

  String json = "{" + lv + tv + wo + fs + id + "}";
  // Serial.print("Signal Created: ");
  // Serial.println(json);
  return json;
}

String Room::sendLight(int lightValue, int roomID){
  String lv = "\"lightVal\": "+ String(temperatureVal);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ lv + id + "}";
  // Serial.print("Signal Created: ");
  // Serial.println(json);
  return json;
}

String Room::sendAngle(int windowAngle, int roomID){
  String wo = "\"windowOpen\": "+ String(windowAngle);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ wo + id + "}";
  // Serial.print("Signal Created: ");
  // Serial.println(json);
  return json;
}

String Room::sendFan(int fanValue, int roomID){
  String fs = "\"fanSpeed\": "+ String(fanValue);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ fs + id + "}";
  // Serial.print("Signal Created: ");
  // Serial.println(json);
  return json;
}

String Room::sendTemp(int temperatureVal, int roomID){
  String tv = "\"tempVal\": "+ String(temperatureVal);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ tv + id + "}";
  // Serial.print("Signal Created: ");
  // Serial.println(json);
  return json;
}

String Room::makeEntrySignal(int openDoor, int lockDoor){
  // {"lightVal": 000, "tempVal": 00, "windowOpen": 000, "fanSpeed": 000}
  String od = "\"doorOpen\": "+ String(openDoor);
  String ld = ", \"doorLocked\": "+ String(lockDoor);

  String json = "{"+ od + ld + "}";
  // Serial.print("Signal Created: ");
  // Serial.println(json);
  return json;
}

void Room::interpretSignal(String sig){
  // Split signals by index
  // Subtract char value '0' so that translation from char to int is correct
  // Multiply by factor of 100 and 10 to make the value correct
  int lv = (100 * (sig[1]- '0') + 10 * (sig[2]- '0') + (sig[3]- '0'));
  int tv = (10 * (sig[4]- '0') + (sig[5]- '0'));
  int wa = (10 * (sig[6]- '0') + (sig[7]- '0'));
  int fs = (100 * (sig[8]- '0') + 10 * (sig[9]- '0') + (sig[10]- '0'));
  int mp = (10 * (sig[11]- '0') + (sig[12]- '0'));

  // Check for changes in value between incoming signal and already set values
  // If different, change value to incoming signal value
  if (lv != lightValue) {setLightValue(lv);} // 000
  if (tv != temperatureVal) {setTemperature(tv);} // 00
  if (wa != winAngle) {setWindowAngleManual(wa);} // 00
  if (fs != fanValue) {setFanValue(fs);} // 000
  if (mp != maxPeople) {setMaxPeople(mp);} // 00    
}
