#include "Room.h"
#include "Request.h"

const char* id = "Hurodor";
const char* pw = "123456789";
const char* baseURL= "http://172.20.10.2:5000/";
const String room = "bedroom";

unsigned long getTimer = millis();
const int getInterval = 5000;

String inputSignal;
String oldInputSignal;
String outputSignal;
String oldOutputSignal = "";


Servo windowServo;
String userID = "1";

Room bedroom(windowServo, userID);
Request r(id, pw, baseURL);

void setup() {
  bedroom.init();
  r.init();
  Serial.begin(115200);
  inputSignal = r.get(room);
  bedroom.interpretSignal(inputSignal);
}

void loop() {

  if (millis() - getTimer > getInterval){
   Serial.println("GET");
   inputSignal = r.get(room);
   getTimer = millis();
   if (inputSignal != oldInputSignal){
     if (inputSignal == "") {inputSignal = oldInputSignal;}
     bedroom.interpretSignal(inputSignal);
     Serial.println(oldInputSignal);
     Serial.println(inputSignal);
     oldInputSignal = inputSignal;
    }
  }
  
  outputSignal = bedroom.makeSignal();
  if (outputSignal != oldOutputSignal){
    Serial.print(outputSignal);
    r.post(room, outputSignal);
    oldOutputSignal = outputSignal;
  }
}
