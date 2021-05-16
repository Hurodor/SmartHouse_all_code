#include <OLED_I2C.h>
#include "Room.h"
#include <ESP32Servo.h>
#include "Request.h"

extern uint8_t SmallFont[];

int i  = 10; //y-posisjon for ">"
int scrollMax = 0;

int page = 0; // sets the main menu as the default page
int nextPage = 0;

int end = 0; // alternative debounce-method
int button1 = 1;
volatile boolean key = false;

int lightingMode[4] = {255, 0, 90, 30}; //mode-values for controlling components
int fanMode[4] = {255, 150, 50, 0};
int servoMode[4] = {90, 60, 30, 0};

const int scrollPin = 5; // button pins
const int selectPin = 4;
const int returnPin = 18;

const int potPin = 13; //potmeter values
int potVal = 0;
const int potMin = 0;
const int potMax = 4095;
const int analogMin = 0;
const int analogMax = 255;

const int tempMin = 15; // temperature values
const int tempMax = 25;
int thermostat = 0;
String thermostatStr = "";

const int angleMin = 0; // servo values
const int angleMax = 90;
int servoAngle = 0;
String servoAngleStr = String(servoAngle);

int brightness = 0; // brightness values
int brightnessPercent = 0;

String percentStr = ""; // string of a percent value

int fanSpeed = 0; // values for controlling the cooling fan
int fanSpeedPercent = 0;

int controlRoomIndx = 0; // Strings which will be displayed in the top of the screen
String controlRoom[4] = {"YOUR ROOM", "LIVING ROOM", "BATHROOM", "ENTRANCE"};

int displayRoomIndx = 0; // Strings which will be displayed in the top of the screen when booking or checking status of a room
String displayRoom[6] = {"Book bathroom","Status bathroom", "Book kitchen","Status kitchen", "Book living room","Status living room"};

// time values for booking/status checking
int fromHour = 0;//(From)
int fromMinute = 0;
int toHour = 0;//(To)
int toMinute = 0;
String Hour1 = "-1";
String Minute1 = "-1";
String Hour2 = "-1";
String Minute2 = "-1";
String status1 = ""; // will be displayed as future bookings of a given room
String status2 = "";
String status3 = "";
String status4 = "";

int bookTo = 1; // decides if the user can set the "to"-time, when they either book or check status of a room
int day = 0; // date (0 for today, 1 for tomorrow), used when booking a room or checking status

int roomID = 0; // Values representing which room the user wants to book, check status for, or control
int bathroomID = 7;
int kitchenID = 8;
int livingroomID = 9;
int bedroomID = 1;

int doorOpen = 0; // these represent the state of the front door 0 for closed/locked, 1 for open/unlocked respectively
int doorLocked = 0;

const int hourMax = 23; // maximum amount of hours in a day
const int minuteMax = 55; // maximum amount of minutes in a day (55 because the user decides the minute value in 5-minute increments)

int confirmTimeValue = 0; // used as parameters when you set the time-frame for booking or status checking
int confirmTimeValueMax = 0; // parameters when the user sets a time for a booking or a status check 

unsigned long lastDebounceTime = 0; // time values for debouncing a button press
unsigned long lastDebounceTime2 = 0; 
unsigned long lastDebounceTime3 = 0;
const int debounceDelay = 350;

const char* ssid = "Hurodor"; // Wifi name
const char* pw = "123456789"; // wifi password. We used "123456789" for simplicity
String baseURL = "http://192.168.43.53:5000/"; // base-URL to/from which the ESP32 will send or gather information
String endPoint = ""; // the end point in the base-URL, used for booking and status checks
String payL = ""; // the String value the database receives

// importing and preparation for the use of the room-and request classes
OLED  myOLED(SDA, SCL);
String idRoom = "1";
String validationCode = "";
Servo s1;
Room r(s1, idRoom);
Request req(ssid, pw, baseURL);

void setup() {
  // put your setup code here, to run once: (would be rude to let this out)
  Serial.begin(115200);

  //req.init(); // initiates wifi-connection

  myOLED.begin(); // initiates the OLED-display
  myOLED.clrScr();
  myOLED.setFont(SmallFont); // fixed string size on the Display

  pinMode(returnPin, INPUT); // initiates the button- and potmeter pins
  pinMode(selectPin, INPUT);
  pinMode(scrollPin, INPUT);
  pinMode(potPin, INPUT);
}

void loop(){
  returnButton();
  selectButton();
  displayPage();
}

void thermo(){ // manually sets the thermostat
  potVal = analogRead(potPin);
  thermostat = map(potVal, potMin, potMax, tempMin, tempMax); // converts the value from the range of potMin-potMax over to tempMin-tempMax 
  thermostatStr = String(thermostat); // thermostat in string-form, used for displaying on the OLED-screen
  delay(100);
}

void servo(){ // manually sets the servo angle, same principle as void thermo()
  potVal = analogRead(potPin);
  servoAngle = map(potVal, potMin, potMax, angleMin, angleMax); // servoAngle can range from 0 to 90 degrees
  servoAngleStr = String(servoAngle);
  delay(100);
}

void setFan(){ // manually sets the fanspeed, again, the same basic principle as in void thermo()
  potVal = analogRead(potPin);
  fanSpeed = map(potVal, potMin, potMax, analogMin, analogMax);
  fanSpeedPercent = map(potVal, potMin, potMax, 0, 100);
  percentStr = String(fanSpeedPercent); // the fanSpeed ranges from 0-255, so it's more useful for the user to see a percentage based value on the screen
  delay(100);
}

void lighting(){ // manually sets the brightness value, same principle used in void setFan()
  potVal = analogRead(potPin);
  brightness = map(potVal, potMin, potMax, analogMin, analogMax);
  brightnessPercent = map(potVal, potMin, potMax, 0, 100);
  percentStr = String(brightnessPercent);
  delay(100);
}

void displayPot(String header, String item1, String itemType, String value){ // displays what value the user is setting with the potmeter
  myOLED.clrScr();
  myOLED.print(header, CENTER, 0); // A header string like for instance "TEMPERATURE"
  myOLED.print(item1, 10,10); // string containing information like "Set thermostat: "
  myOLED.print(value, CENTER, 25); // "value" if the value derived from one of the four previous functions, for instance, the thermostatStr
  myOLED.print(itemType, 80, 25); // unit of measurement, like for instance " deg. C"
  myOLED.update();
}

void scrollButton(){ // gives the user a sense of scrolling, as the ">" moves on the screen 
  bool x = digitalRead(scrollPin);

  if (x == false){ // if the user doesnt press the scroll button, nothing will happen
    button1 = 0;
  }

  if (x == true && button1 == 0){ // When x = high --> both end and key will be set to 1, which lets the following if statement run
    end = 1;
    key = true;
  }
    
  if (key == true && end == 1){// Both key and end are high. The ">" will then move 15 pixels down
    button1 = 1; 
    myOLED.clrScr();
    myOLED.update();
    i = i + 15;
    key = false; 
    end = 0;
  }
    
  if (i > scrollMax){
    i = 10;
  }
}

void printMenu(String header, String item1, String item2, String item3, String item4){ // easier way to display a menu
  myOLED.clrScr();
  myOLED.print(header, CENTER, 0); // Header is in all-caps in the top of the screen
  myOLED.print(item1, 10, 10); // item1-item4 are sub menus a user can scroll down to, and subsequently go in to
  myOLED.print(item2, 10, 25);
  myOLED.print(item3, 10, 40); // the screen can display a maximum of 5 lines, but if a sub-menu contains less than that, itemX can be set to "" (nothing)
  myOLED.print(item4, 10, 55); // since the scrollMax is set for every page, it doesnt matter that an item will be displayed as "", because the user cant scroll to it anyways

  myOLED.print(">", 0, i); // what the user "scrolls"
  myOLED.update();
  scrollButton(); // the function which updates the y-position of ">" on a button press
}

void returnButton(){ // reliably sets the page to the main menu on a button press of the return-button 
  int back = digitalRead(returnPin);
  if (back == 1){
    i = 10;
    page = 0;
    confirmTimeValue = 0;
  }
}

void selectDay(){ // choose day to either book or check status for
    scrollMax = 25;
    printMenu("VELG DAG", "I dag", "I morgen", "", "");
}

void displayPage(){ // displays what the user will see on the screen
  switch(page){
    case 0: // main menu
      scrollMax = 55; // maximum y-position for ">", is used in almost every page
      printMenu("MAIN MENU", "Resources", "Lighting", "Cooling", "Heating");
      break;
    case 100: // resources
      scrollMax = 40;
      printMenu("RESOURCES", "Book resource", "Check status", "Room control", "");
      break;
      case 101: // choose day for booking
        selectDay();
        break;
      case 102: // choose day for status check
        selectDay();
        break;
      case 110: // book
        endPoint = "booking";
        scrollMax = 40;
        printMenu("BOOK RESOURCE", "Bathroom", "Kitchen", "Living room", "");
        break;

        case 111: // book bathroom
          bookRoom();
          break;

        case 112: // book kitchen
          bookRoom();
          break;

        case 113: // book living room
          bookRoom();
          break;
      case 120: // check status
        endPoint = "booking"; // shares endpoint with booking
        scrollMax = 40;
        printMenu("CHECK STATUS", "Bathroom", "Kitchen", "Living room", "");
        break;

        case 121: // check status for a room
          statusRoom();
          break;
          case 1211: // displays the bookings for a given room
            myOLED.clrScr();
            myOLED.print("BOOKINGS", CENTER,0);
            myOLED.print(status1, 0, 10);
            myOLED.print(status2, 0, 25);
            myOLED.print(status3, 0, 40);
            myOLED.print(status4, 0, 55);
            myOLED.update();
            break;

      case 130: // room control
        scrollMax = 40;
        printMenu("ROOM CONTROL", "Living room", "Bathroom", "Entrance", "");
        break;
        case 131: // control a given room, controlRoom[] is an array containing room names, where the index is updated, so room control for any room shares the same page
          printMenu(controlRoom[controlRoomIndx], "Lighting", "Cooling", "Heating", "");
          break;
        case 133: // control the front door
          scrollMax = 25;
          printMenu("ENTRANCE", "Door", "Lock", "", "");
          break;
          case 1331:
            scrollMax = 25;
            printMenu("DOOR", "Open door", "Close door", "", "");
            break;
          case 1332:
            scrollMax = 25;
            printMenu("LOCK", "Unlock door", "Lock door", "", "");
            break;
    case 200: // lighting
      scrollMax = 40;
      printMenu(controlRoom[controlRoomIndx], "Manual lighting", "Automode", "Choose mode", "");
      break;
      case 210: // manual lighting
        lighting();
        displayPot("Lighting", "Sett value: ", "%", percentStr);
        break;
      case 220: // automatic mode - doesnt actually do anything yet, as this is a proof of concept
        myOLED.clrScr();
        myOLED.print("You chose automode,", CENTER, 10);
        myOLED.print("press return.", CENTER, 25);
        myOLED.update();
        break;
      case 230: // choose a preset mode
        scrollMax = 55;
        printMenu("LIGHT MODE", "On", "Off", "Day", "Movie");
        break;
    case 300: // cooling 
      scrollMax = 25;
      printMenu(controlRoom[controlRoomIndx], "Fan", "Servo", "", "");
      break;
      case 310: // fan
        scrollMax = 40;
        printMenu("FAN", "Manual", "Automatic", "Choose mode", "");
        break;
        case 311: // manual fanspeed
          setFan();
          displayPot("FAN", "Set fanspeed: ", "%", percentStr);
          break;
        case 312: // automatic fan
          myOLED.clrScr();
          myOLED.print("You chose automode,", CENTER, 10);
          myOLED.print("press return.", CENTER, 25);
          myOLED.update();
          break;
        case 313: // preset modes for fanspeed
          scrollMax = 55;
          printMenu("FAN MODE", "High", "Medium", "Low", "Off");
          break;
      case 320: // Servo
        scrollMax = 40;
        printMenu("SERVO", "Manual", "Automatic", "Choose mode", "");
        break;
        case 321: // manual servo
          servo();
          displayPot("SERVO", "Set angle: ", "deg.", servoAngleStr);
          break;
        case 322: // automatic servo
          myOLED.clrScr();
          myOLED.print("You chose automode,", CENTER, 10);
          myOLED.print("press return.", CENTER, 25);
          myOLED.update();
          break;
        case 323: // preset servo angle
          scrollMax = 55;
          printMenu("SERVO MODE", "90 degrees", "60 degrees", "30 degrees", "0 degrees");
          break;
      case 400: // manual thermostat
        Serial.println(thermostatStr);
        thermo();
        displayPot("TEMPERATURE", "Set value: ", "deg. C", thermostatStr);
        nextPage = 410;
        break;
  }
}

void nextDisplayPage(){ // the selectButton-function uses the page the user is at and the y-position of the ">", to determine what the next page will be
  switch(page){
      case 0: // main menu, nextPage is where the user will end up upon a button-press
        switch(i){
          case 10: // resources
            nextPage = 100;
            break;
          case 25: // lighting
            endPoint = "roomcontrol";
            roomID = bedroomID;
            controlRoomIndx = 0;
            nextPage = 200;
            break;
          case 40: // cooling
            endPoint = "roomcontrol";
            roomID = bedroomID;
            controlRoomIndx = 0;
            nextPage = 300;
            break;
          case 55: // heating
            endPoint = "roomcontrol";
            roomID = bedroomID;
            nextPage = 400;
        }
        break;
      case 100: // resources
        switch(i){
          case 10:
            nextPage = 101; // choose day for booking
            break;
          case 25: 
            nextPage = 102; // choose day for status
            break;
          case 40:
            nextPage = 130; // room control
            break;
        }
        break;
      
        case 101: // choose day for booking
          switch(i){
            case 10: // today
              day = 0;
              nextPage = 110;
              break;
            case 25: // tomorrow
              day = 1;
              nextPage = 110;
              break;
          }
          break;
        case 102: // choose day for status
          switch(i){
            case 10: // today
              day = 0;
              nextPage = 120;
              break;
            case 25: // tomorrow
              day = 1;
              nextPage = 120;
              break;
          }
          break;
        case 110: //book for a given day
          switch(i){
            case 10:
              displayRoomIndx = 0;
              roomID = bathroomID;
              nextPage = 111; // bathroom
              break;
            case 25:
              displayRoomIndx = 2;
              roomID = kitchenID;
              nextPage = 112; // kitchen
              break;
            case 40:
              displayRoomIndx = 4;
              roomID = livingroomID;
              nextPage = 113; // living room
              break;
          }
          break;
        case 120: // check status for given day
          switch(i){
            case 10:
              displayRoomIndx = 1;
              roomID = bathroomID;
              nextPage = 121; // bathroom
              break;
            case 25:
              displayRoomIndx = 3;
              roomID = kitchenID;
              nextPage = 121; // kitchen
              break;
            case 40:
              displayRoomIndx = 5;
              roomID = livingroomID;
              nextPage = 121; // living room
              break;
          }
          break;
          case 121: // checks status for a given room
          {
            Serial.print(endPoint);
            Serial.println(payL);
            getBookings(); // gathers future bookings
            nextPage = 1211;
            break;

          }
        case 130: // room control
          endPoint = "roomcontrol";
          switch(i){
            case 10: // living room
              controlRoomIndx = 1;
              roomID = livingroomID;
              nextPage = 131;
              break;
            case 25: // bathroom
              controlRoomIndx = 2;
              roomID = bathroomID;
              nextPage = 131;
              break;
            case 40: // entrance
              controlRoomIndx = 3;
              nextPage = 133;
              break;
          }
          break;
          case 131: // control a given room
            switch(i){
              case 10:
                nextPage = 200; //lighting
                break;
              case 25: // cooling
                nextPage = 300;
                break;
              case 40: // heating
                nextPage = 400;
                break;
            }
            break;
          
          case 133: // front door
            endPoint = "entry";
            switch(i){
              case 10: // door
                nextPage = 1331;
                break;
              case 25: // lock
                nextPage = 1332;
                break;
            }
            break;
          case 1331: // door
            switch(i){
              case 10: // open door
                doorOpen = 1;
                nextPage = 13311;
                break;
              case 25: // close door
                doorOpen = 0;
                nextPage = 13311;
                break;
            }
            break;
          case 1332: // lock
            switch(i){
              case 10: // unlock door
                doorLocked = 0;
                nextPage = 13311;
                break;
              case 25: // lock door
                doorLocked = 1;
                nextPage = 13311;
                break;
            }
            break;
          case 13311: // sends information to the database about door-information
            {
            payL = r.makeEntrySignal(doorOpen, doorLocked);
            Serial.print(endPoint);
            Serial.println(payL);
            req.put(endPoint, payL);
            nextPage = 0;
            break;
            }

        case 200: // lighting
          switch(i){
            case 10: // manual lighting
              nextPage = 210;
              break;
            case 25: // automatic mode
              nextPage = 220;
              break;
            case 40: // choose preset mode
              nextPage = 230;
          }
          break;
          case 210: // send light values to the database
          {
            payL = r.sendLight(brightness, roomID);
            Serial.print(endPoint);
            Serial.println(payL);
            req.put(endPoint, payL);
            nextPage = 0;
            break;
          }
          case 230: // lets the user decide the preset lighting mode
            {
            switch(i){
              case 10: // on
                payL = r.sendLight(lightingMode[0], roomID);
                break;
              case 25: // off
                payL = r.sendLight(lightingMode[1], roomID);
                break;
              case 40: // day
                payL = r.sendLight(lightingMode[2], roomID);
                break;
              case 55: // movie (dimmed light)
                payL = r.sendLight(lightingMode[3], roomID);
                break;
            }
            req.put(endPoint, payL);
            Serial.print(endPoint);
            Serial.println(payL);
            nextPage = 0;
            }
            break;
            
        case 300: // cooling
          switch(i){
            case 10: // fan
              nextPage = 310;
              break;
            case 25: // servo
              nextPage = 320;
              break;
          }
          break;
          case 310: // fan
            switch(i){
              case 10: // manual fanspeed
                nextPage = 311;
                break;
              case 25: // automatic fan
                nextPage = 312;
                break;
              case 40: // fan mode
                nextPage = 313;
            }
            break;
            case 311: // send manually set fanspeed
            {
              payL = r.sendFan(fanSpeed, roomID);
              Serial.print(endPoint);
              Serial.println(payL);
              req.put(endPoint, payL);
              nextPage = 0;
              break;
            }
            break;
            case 313: // choose fan mode
              {
              switch(i){
                case 10: // high
                  payL = r.sendFan(fanMode[0], roomID);
                  break;
                case 25: // medium
                  payL = r.sendFan(fanMode[1], roomID);
                  break;
                case 40: // low
                  payL = r.sendFan(fanMode[2], roomID);
                  break;
                case 55: // off
                  payL = r.sendFan(fanMode[3], roomID);
                  break;
              }
              req.put(endPoint, payL);
              Serial.print(endPoint);
              Serial.println(payL);
              Serial.println(validationCode);
              nextPage = 0;
              }
              break;
              
          case 320: // servo
            switch(i){
              case 10: // manual servo
                nextPage = 321;
                break;
              case 25:
                nextPage = 322; // automatic servo
                break;
              case 40:
                nextPage = 323; // choose servomode
                break;
            }
            break;
            case 321: // send manually set servo angle
              {
              payL = r.sendAngle(servoAngle, roomID);
              Serial.print(endPoint);
              Serial.println(payL);
              req.put(endPoint, payL);
              nextPage = 0;
              break;
            }
            case 323: // choose preset servo angle
              {
              switch(i){
                case 10: // 90°
                  payL = r.sendAngle(servoMode[0], roomID);
                  break;
                case 25: // 60°
                  payL = r.sendAngle(servoMode[1], roomID);
                  break;
                case 40: // 30°
                  payL = r.sendAngle(servoMode[2], roomID);
                  break;
                case 55: // 0°
                  payL = r.sendAngle(servoMode[3], roomID);
                  break;
              }
              req.put(endPoint, payL);
              Serial.print(endPoint);
              Serial.println(payL);
              nextPage = 0;
              }
              break;
        case 400: // send manually set thermostat value
        {
          payL = r.sendTemp(thermostat, roomID);
          req.put(endPoint, payL);
          Serial.print(endPoint);
          Serial.println(payL);
          nextPage = 0;
          break;
        }
        default:
          break;
      }
}

void selectButton(){ // sets the next page-value when the select button is pressed
  int selMeny = digitalRead(selectPin); // Select-button pin reading
  
  if(millis()-lastDebounceTime > debounceDelay && selMeny == 1){ // button debouncing
    nextDisplayPage();
    page = nextPage; // sets the page value to the nextPage, which updates the page
    lastDebounceTime = millis();
    i = 60; // since i will never be 60, the scrollButton()-function will set i = 10 every time you enter a new page
    }
}

void bookingDisplay(){ // displays what the user is doing while either booking a room or checking status
  bookFrom();
  Hour1 = String(fromHour);
  Minute1 = String(fromMinute);
  Hour2 = String(toHour);
  Minute2 = String(toMinute);

  if(fromHour<10){Hour1 = "0" + Hour1;}// this row of if statements checks and corrects the format of the time values
  if(fromMinute<10){Minute1 = "0" + Minute1;}
  if(toHour<10){Hour2 = "0" + Hour2;}
  if(toMinute<10){Minute2 = "0" + Minute2;}
 
  myOLED.clrScr();
  myOLED.print(String(displayRoom[displayRoomIndx]), CENTER, 0);

  myOLED.print("From: ", 10, 10);
  myOLED.print(Hour1, 60, 10);  myOLED.print(":",75,10);  myOLED.print(Minute1,82,10);

  if(bookTo == 1){ // only runs when a user is booking
    myOLED.print("To: ", 10, 25);
    myOLED.print(Hour2,60,25);  myOLED.print(":",75,25);  myOLED.print(Minute2,82,25);
  }

  myOLED.update();
}

void bookRoom(){ // books a room
  bookTo = 1;
  confirmTimeValueMax = 6;
  if(confirmTimeValue != 5 && confirmTimeValue != -1){
    bookingDisplay();
  }

  if(confirmTimeValue == 5){ // confirmTimeValue = 5 when the user has selected all time-values
      r.setStartHour(Hour1, Minute1);
      r.setEndHour(Hour2, Minute2);
      r.bookResource(roomID, day, r.getStartHour(),r.getEndHour());
      Serial.println(r.getBookingString());
      validationCode = req.post("booking", r.getBookingString());
      Serial.println(validationCode);
      confirmTimeValue = -1;
      Hour1 = "-1"; Minute1 = "-1"; Hour2 = "-1"; Minute2 = "-1"; // prepares for next booking
      fromHour = 0; fromMinute = 0; toHour = 0; toMinute = 0;
  }

    if(confirmTimeValue == -1){ // tells the user that the booking has been sent
      myOLED.clrScr();
      myOLED.print("Booking sent!", CENTER, 0);
      myOLED.print(validationCode, CENTER, 30); // validationCode is a string the ESP receives from the database
      myOLED.update();
    }
}

void statusRoom(){ // checks the status for a given room
  bookTo = 0;
  confirmTimeValueMax = 3;
  if(confirmTimeValue != confirmTimeValueMax && confirmTimeValue != -1){
    bookingDisplay();
  }

  if(confirmTimeValue == confirmTimeValueMax){ // prepares for next status check
      confirmTimeValue = -1;
      Hour1 = "-1";
      Minute1 = "-1";
      Hour2 = "-1";
      Minute2 = "-1";
      fromHour = 0; fromMinute = 0; toHour = 0; toMinute = 0;
  }
}

void bookFrom(){ // lets the user select a time frame
  int scrollTime = digitalRead(scrollPin); // Scroll
  int confirmTime = digitalRead(selectPin); // Confirm
  int confirmTimeValueMax; // decides what time value is changed by the user
  
  if(millis()-lastDebounceTime2 > debounceDelay && confirmTime == 1){ // debouncing the addition of +1 to confirmTimeValueMax
    confirmTimeValue = confirmTimeValue + 1;
    lastDebounceTime2 = millis();
    Serial.println(confirmTimeValue);
    if(confirmTimeValue==confirmTimeValueMax){
      confirmTimeValue=0;}
  }

  if(millis()-lastDebounceTime3 > debounceDelay && scrollTime == 1){ // debouncing a scroll button press
    switch(bookTo){ // decides if the user is booking or checking status
    case 1: // booking
      confirmTimeValueMax = 6;
      switch(confirmTimeValue){
      case 1:
        fromHour += 1;
        toHour = fromHour;
        if (fromHour > hourMax){
          fromHour = 0;
        }
        break;
      
      case 2:
        fromMinute += 5;
        toMinute = fromMinute;
        if(fromMinute > minuteMax){
          fromMinute = 0;
        }
        break;

      case 3:
        toHour += 1;
        if (toHour < fromHour || toHour > hourMax){
          toHour = fromHour;
        }
        break;

      case 4:
        toMinute = toMinute + 5;
        if(toMinute > minuteMax){
          toMinute = 0;
        }
        if(toHour == fromHour && toMinute < fromMinute){
          toMinute = fromMinute + 5;
        }
        break;
      }
      break;

      // the if statements within the cases forces the user to select valid values

    case 0: // status
      confirmTimeValueMax == 3;
      switch(confirmTimeValue){
      case 1:
        fromHour += 1;
        if (fromHour > hourMax){
          fromHour = 0;
        }
        break;

      case 2:
        fromMinute += 5;
        if (fromMinute > minuteMax){
          fromMinute = 0;
        }
        break;
      }
    default:
      confirmTimeValue = -1;
      confirmTimeValueMax = 0;
      break;
    }
  lastDebounceTime3 = millis();     
  }
}

void bookFrom(){ // lets the user select a time frame
  int scrollTime = digitalRead(scrollPin); // Scroll
  int confirmTime = digitalRead(selectPin); // Confirm
  int confirmTimeValueMax; // decides what time value is changed by the user
  
  if(millis()-lastDebounceTime2 > debounceDelay && confirmTime == 1){ // debouncing the addition of +1 to confirmTimeValueMax
    confirmTimeValue = confirmTimeValue + 1;
    lastDebounceTime2 = millis();
    Serial.println(confirmTimeValue);
    if(confirmTimeValue==confirmTimeValueMax){
      confirmTimeValue=0;}
  }

  if(millis()-lastDebounceTime3 > debounceDelay && scrollTime == 1){ // debouncing a scroll button press
    switch(bookTo){ // decides if the user is booking or checking status
    case 1: // booking
      confirmTimeValueMax = 6;
      switch(confirmTimeValue){
      case 1: // increases the from-hour
        fromHour += 1;
        toHour = fromHour;
        if (fromHour > hourMax){
          fromHour = 0;
        }
        break;
      
      case 2: // increases the from-minutes
        fromMinute += 5;
        toMinute = fromMinute;
        if(fromMinute > minuteMax){
          fromMinute = 0;
        }
        break;

      case 3:
        toHour += 1;
        if (toHour < fromHour || toHour > hourMax){
          toHour = fromHour;
        }
        break;

      case 4:
        toMinute = toMinute + 5;
        if(toMinute > minuteMax){
          toMinute = 0;
        }
        if(toHour == fromHour && toMinute < fromMinute){
          toMinute = fromMinute + 5;
        }
        break;
      }
      break;

      // the if statements within the cases forces the user to select valid values

    case 0: // status
      confirmTimeValueMax == 3;
      switch(confirmTimeValue){
      case 1: // increases the from-hour
        fromHour += 1;
        if (fromHour > hourMax){
          fromHour = 0;
        }
        break;

      case 2: // increases the from-minute
        fromMinute += 5;
        if (fromMinute > minuteMax){
          fromMinute = 0;
        }
        break;
      }
    default:
      confirmTimeValue = -1;
      confirmTimeValueMax = 0;
      break;
    }
  lastDebounceTime3 = millis();     
  }
}