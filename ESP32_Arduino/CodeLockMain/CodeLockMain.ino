
int button1 = 12;
int button2 = 11;
int button3 = 10;
int button4 = 9;
int button5 = 8;
int button6 = 7;
int button7 = 6;
int button8 = 5;
int button9 = 4;
int greenLED = 13;
int redLED = 2;

//Pin number to buttons

String okCode = "1234"; //Correct code
String tryCode = ""; //String that check if equal to "okCode"



void setup() {

  Serial.begin (9600); // Use Arduino, not ESP32
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  pinMode(button5, INPUT);
  pinMode(button6, INPUT);
  pinMode(button7, INPUT);
  pinMode(button8, INPUT);
  pinMode(button9, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

}

void loop() {
  
  if(digitalRead(button1) == 1){
    pressButton(button1, "1", tryCode);
    }
    
  else if(digitalRead(button2) == 1){
    pressButton(button2, "2", tryCode);
  }

  else if(digitalRead(button3) == 1){
    pressButton(button3, "3", tryCode);
  }
  else if (digitalRead(button4) == 1){
    pressButton(button4, "4", tryCode);
  }
  
  else if (digitalRead(button5) == 1){
    pressButton(button5, "5", tryCode);
  }

  else if (digitalRead(button6) == 1){
    pressButton(button6, "6", tryCode);
  }

  else if (digitalRead(button7) == 1){
    pressButton(button7, "7", tryCode);
  }

  else if (digitalRead(button8) == 1){
    pressButton(button8, "8", tryCode);
  }
  else if (digitalRead(9) == 1){
    pressButton(button9, "9", tryCode);
  }
//Give different buttons different value
  

  if(tryCode.length() == 4 && tryCode == okCode){ //If the code is correct
    for(int time = 0; time < 10; time++){ //Blink 10 times
      digitalWrite (greenLED, HIGH);
      delay(50);
      digitalWrite(greenLED, LOW); //A green LED will blink
      delay(50);
    }
    tryCode = ""; //If not pushed, a red LED will light
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);

  }
  if(tryCode.length() == 4 && tryCode != okCode){ //If the code is wrong
    for (int time = 0; time < 20; time++){ //Blink 20 times
      digitalWrite(redLED, HIGH);
      delay(50);
      digitalWrite(redLED, LOW);
      delay(50);
    }
    tryCode = "";
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
  }
  else{
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
  }  

}

void pressButton(int butPin, String Value, String& tryCode){ //Function that takes pin, if HIGH or LOW and reference to the pushed code
  unsigned long timer = millis();
  while (digitalRead(butPin) == 1){} //Empty while loop. If you hold a button, only one value will be given
  if(millis()- timer > 100){ // Debounce so no buttons will give value unless they are pressed
    tryCode = tryCode + Value; //Puts the number value to tryCode
  }
}
