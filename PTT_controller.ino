// AJ Morris
// M0CWX
// Sept 2021
// m0cwx@qsl.net

// PTT Controller checks whether PTTPin is open or closed and
// only tells the IF rig to TX if the Transverter is already set
// to TX after SafetyDelay amount of time
// When PTTPin is open, then the IF is switched to RX before the 
// Transverter by SafetyDelay amount of time.
//
// Rather than just having a steady "power on" light, the LED 
// heartbeats at frequency SleepTime*heartbeat/fadeAmount
// (give or take rounding)...
// TODO: think about whether this rounding  is "floor" or "ceiling".

// Things you might want to fiddle with
const int SerialBaud = 19200; // How fast do you need it ? 
const int SafetyDelay = 500;  // How long before IF is safe before TXVR moved to RX in ms

int SleepTime=100;  // Miliseconds between checking PTT status. Why work harder than you have to?
int heartbeat=250;  // Brightest the Heartbeat LED gets 250/255
int fadeAmount=25;  // How much the Heartbeat LED changes per SleepTime

const int debounceDelay = 10; // milliseconds to wait until PTT stable

// Things you only want to fiddle with if you have a different 
// microprocessor
const int PTTPin = 2; // PTT from Mic
const int TXVRPin = 3; // PTT to Transverter
const int IFPin = 5;  // PTT to Rig
const int TXVRMonitorPin = 7; // LED monitor
const int IFMonitorPin = 8; // LED monitor
const int HeartBeatPin = 9;  // Analogue

// Don't need to fiddle with these.
byte TXStatus = 0; // 0 is RX, 1 is TX
int TimeSinceSwitch; // in SleepTime ms


// S E T U P
void setup() {

  Serial.begin(SerialBaud);
  
  pinMode(PTTPin, INPUT); // PTT
  pinMode(TXVRPin, OUTPUT); // Transverter
  pinMode(IFPin, OUTPUT); // IF
  pinMode(IFMonitorPin, OUTPUT); // IF LED on
  pinMode(TXVRMonitorPin, OUTPUT); // IF Transverter on
  pinMode(HeartBeatPin, OUTPUT); // Heartbeat
  Serial.print("\n\n\n\nON\n");

  TXStatus=0; // Start in RX mode.
  TimeSinceSwitch=0; // reset counter.
 
}

// From the internet. Makes sure that that the input pin has 
// remained in its current state for debounceDelay ms.
// https://www.arduinoplatform.com/reliable-detecting-a-switch-with-arduino/
boolean debounce(int pin)
{
 boolean state;
 boolean previousState;
 previousState = digitalRead(pin); // store switch state
 for(int counter=0; counter < debounceDelay; counter++)
 {
 delay(1); // wait for 1 millisecond
 state = digitalRead(pin); // read the pin
 if( state != previousState)
 {
 counter = 0; // reset the counter if the state changes
 previousState = state; // and save the current state
 }
 }
 // here when the switch state has been stable longer than the debounce period
 return state;
}


// S W I T C H   T O   T X 
void switch_TX() {
  TXStatus = 1;
  transverter(1);
  delay(SafetyDelay);
  IF(1);
}

// S W I T C H   T O   R X 
void switch_RX() {
  TXStatus = 0;
  IF(0);
  delay(SafetyDelay);
  transverter(0);
}

// S W I T C H   T O   I F  R E L A Y
void IF(byte x) {
 if (x==1) {
    digitalWrite(IFPin, HIGH);
    Serial.print("Set IF to TX\n\r");
    digitalWrite(IFMonitorPin, HIGH); 
 }
 else if (x==0) {
  digitalWrite(IFPin, LOW);
  Serial.print("Set IF to RX\n\r"); 
  digitalWrite(IFMonitorPin, LOW); 
 }
}

// S W I T C H   T O  T R A N S V E R T E R   R E L A Y
void transverter(byte x) {
 if (x==1) {
   digitalWrite(TXVRPin, HIGH);
   Serial.print("Set Transverter to TX\n\r"); 
   digitalWrite(TXVRMonitorPin, HIGH);
 }
 else if (x==0) {
   digitalWrite(TXVRPin, LOW);
   Serial.print("Set Transverter to RX\n\r"); 
   digitalWrite(TXVRMonitorPin, LOW);
 }
}

// M A I N   L O O P 
void loop() {

  analogWrite(HeartBeatPin, heartbeat);   // Write the current heartbeat intensity
  Serial.print("\r"); // We've already done \n\r if anything happened last cycle
  // so we just overwrite the status report numbers if nothing happened.
  Serial.print(TXStatus); // Write whether we are RX=0 or TX=1
  Serial.print(" "); 
  Serial.print(TimeSinceSwitch); // Time since switch on in units of SleepTime ms.
  //TODO: Learn how to concatenate Serial.print() arguments!
   
  if ((!TXStatus) && (debounce(PTTPin)==LOW)){ 
    // We are on RX and the PTT has been closed.
   TimeSinceSwitch=0; // reset time since PTT switched
   Serial.print("PTT Closed\n\r"); 
   switch_TX();
  }
  else if ((TXStatus) && (debounce(PTTPin)==HIGH)){
    // We are on TX and the PTT has been opened.
   TimeSinceSwitch=0; // reset time since PTT switched
   Serial.print("PTT Open\n\r"); 
   switch_RX();
  }

 delay(SleepTime); // maybe we shouldn't be constantly checking the PTT.
 heartbeat = heartbeat + fadeAmount; // Change the intensity of the heartbeat pin
 if (heartbeat <= 0 ) {
   // Check the limits of the heartbeat pin 
   // Go between [0, 250] out of max 255
   fadeAmount = -fadeAmount; // Now fade up.
   heartbeat=0; // Can't go lower than 0.
 }
 else if(heartbeat >= 250) {
   fadeAmount = -fadeAmount; // Now fade down.
   heartbeat=250;// Can't go higher than 255.
  }
  
 TimeSinceSwitch++; // Time marches on, ++ at a time.
}
