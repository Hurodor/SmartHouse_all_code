// The direction sensor is a stand-alone circuit which reads the direction of anything
// passing it and sends a signal with information about the direction.
// --------------------------------------------------------------------------------//
// Pins.

const int pot1 = A0;
const int pot2 = A1;
const int leavingOutputPin = 4;
const int enteringOutputPin = 3;

//--------------------------------------------------------------------------------//

// Input values on potMeters.
int p1Val;
int p2Val;
// Threshold values.
int p1Thresh;
int p2Thresh;

//--------------------------------------------------------------------------------//
// Timer used for calculating direction.
unsigned long timer;

//--------------------------------------------------------------------------------//
// Boolean variables.

bool p1Passed;
bool p2Passed;
bool leaving = false;
bool entering = false;
bool firstPrint;

//--------------------------------------------------------------------------------//

void setup() {
  // General setup.
  Serial.begin(9600);
  pinMode(pot1, INPUT);
  pinMode(pot2, INPUT);
  pinMode(enteringOutputPin, OUTPUT);
  pinMode(leavingOutputPin, OUTPUT);

  //--------------------------------------------------------------------------------//
  // Get average light value over a certain time, and set the threshold values to
  // 100 points above this. This reduces the chance of random shifts in light values
  // producing signals.
  
  for (int i = 0; i < 25; ++i){
    p1Thresh += analogRead(pot1);
    p2Thresh += analogRead(pot2);
    delay(50);
  }
  p1Thresh = 100 + (p1Thresh / 25);
  p2Thresh = 100 + (p2Thresh / 25);

  //--------------------------------------------------------------------------------//
  // Debugging print statements, useful in case of weird signals.

  Serial.println("Setup complete");
  Serial.print("P1 threshold = ");
  Serial.println(p1Thresh);
  Serial.print("P2 threshold = ");
  Serial.println(p2Thresh);
}

//--------------------------------------------------------------------------------//

void loop() {
  leaving = false;
  entering = false;
  p1Val = analogRead(pot1);
  p2Val = analogRead(pot2);

  //--------------------------------------------------------------------------------//
  // Resets system after 500ms if only one potmeter has been passed.
  if ((millis() - timer) > 500){
    p1Passed = false;
    p2Passed = false;
  }

  //--------------------------------------------------------------------------------//

  // Checks if the value on potMeter1 is above threshold.
  if (p1Val > p1Thresh){
    
    // Checks if potMeter2 has already been passed within the last 500ms.
    if (p2Passed){

      // If so, the direction is leaving, not entering. PotMeter2 is reset.
      // FirstPrint is only used for debugging and ensuring the direction
      // is only printed once to Serial Monitor.
      leaving = true;
      entering = false;
      p2Passed = false;
      firstPrint = true;
    }
    else{

      // If not, simply potMeter1 has been passed.
      p1Passed = true;
    }

    // Update the timer value to ensure resetting after 500ms.
    timer = millis();
  }

  // Check same for potMeter2.
  else if (p2Val > p2Thresh){
    if (p1Passed){
      leaving = false;
      entering = true;
      p1Passed = false;
      firstPrint = true;
    }
    else{
      p2Passed = true;
    }
    timer = millis();
  }

  //--------------------------------------------------------------------------------//
  // Write signal to output pins.

  digitalWrite(leavingOutputPin, leaving);
  digitalWrite(enteringOutputPin, entering);

  //--------------------------------------------------------------------------------//
  // Debugging print statements.
  
  if (leaving && firstPrint){
    Serial.println("Leaving");
    firstPrint = !firstPrint;
  }
  else if (!leaving && firstPrint){
    Serial.println("Entering");
    firstPrint = !firstPrint;
  }
}
