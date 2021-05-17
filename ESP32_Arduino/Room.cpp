#include "Room.h"

Room::Room(Servo& ws, String& userID)
    :windowServo{ws}, userID{userID}{
    
    enablePin = 14;
    ledPin = 5;
    ledPinOven = 18;
    windowServoPin = 16;
    buzzerGPIOpin = 27;
    startHour = "HH:MM:SS";
    endHour = "HH:MM:SS";
}

void Room::bookResource(int roomNumber, int date, String startHour, String endHour){
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
    if (numPeople > 0){--numPeople;}
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

String Room::setDoorStatus(bool doorOpen, bool locked){
    String sig = "{\"doorOpen\": " + String(doorOpen) + ", \"doorLocked\": " + String(locked) + "}";
    return sig;
}

String Room::makeSignal(){
  String lv = "\"lightVal\": " + String(lightValue);
  String tv = ", \"tempVal\": " + String(temperatureVal);
  String wo = ", \"windowOpen\": " + String(winAngle);
  String fs = ", \"fanSpeed\": " + String(fanValue);
  String id = ", \"roomID\": " + String(userID);

  String json = "{" + lv + tv + wo + fs + id + "}";
  return json;
}

String Room::sendLight(int lightValue, int roomID){
  String lv = "\"lightVal\": "+ String(temperatureVal);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ lv + id + "}";
  return json;
}

String Room::sendAngle(int windowAngle, int roomID){
  String wo = "\"windowOpen\": "+ String(windowAngle);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ wo + id + "}";
  return json;
}

String Room::sendFan(int fanValue, int roomID){
  String fs = "\"fanSpeed\": "+ String(fanValue);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ fs + id + "}";
  return json;
}

String Room::sendTemp(int temperatureVal, int roomID){
  String tv = "\"tempVal\": "+ String(temperatureVal);
  String id = ", \"roomID\": "+ String(roomID);

  String json = "{"+ tv + id + "}";
  return json;
}

String Room::makeEntrySignal(int openDoor, int lockDoor){
  String od = "\"doorOpen\": "+ String(openDoor);
  String ld = ", \"doorLocked\": "+ String(lockDoor);

  String json = "{"+ od + ld + "}";
  return json;
}

void Room::interpretSignal(String sig){
  int lv = (100 * (sig[1]- '0') + 10 * (sig[2]- '0') + (sig[3]- '0'));
  int tv = (10 * (sig[4]- '0') + (sig[5]- '0'));
  int wa = (10 * (sig[6]- '0') + (sig[7]- '0'));
  int fs = (100 * (sig[8]- '0') + 10 * (sig[9]- '0') + (sig[10]- '0'));
  int mp = (10 * (sig[11]- '0') + (sig[12]- '0'));

  if (lv != lightValue) {setLightValue(lv);} 
  if (tv != temperatureVal) {setTemperature(tv);}
  if (wa != winAngle) {setWindowAngleManual(wa);}
  if (fs != fanValue) {setFanValue(fs);}
  if (mp != maxPeople) {setMaxPeople(mp);}    
}
