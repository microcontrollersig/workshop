/*
 *  Curtain_DC_7--TinkerCad is designed to use a DC Motor 
 *   and limit switches to open and close curtains. 
 *   It was developed and debuged using TinkerCad
 
CHANGES to v3
1. Can now press and hold button rather than quick press as before.
   Inconvenient for Sandy.


CHANGES to v2

1. backoff too quick, put delay before reversing
2. give ability to interrupt opening and closing
3. in IDLE state, allow either open or close button

When the program starts, the state of the curtains
(fully opened, partially open, fully closes) is unknown.

There is a state diagram that explains the transitions visually

https://github.com/microcontrollersig/workshop/tree/master/john


*/


//------------------------------ ARDUINO NANO PIN CONNECTIONS----------------------------//
// Motor B (arduino pins 3,5,4 for enable pin, direction pins 4 and 5)

const int enB = 3;
const int in3 = 5;
const int in4 = 4;

// Limit switches (arduino pins 7 and 8 for open and close limit switches)
const int openLimitPin = 7;
const int closeLimitPin = 8;

// Buttons (arduino pins 11 and 12)
const int openButtonPin = 11;
const int closeButtonPin = 12;
//------------------------------ END ARDUINO NANO PIN CONNECTIONS----------------------------//

/*

PROGRAM STATE:

IDLE: Program's initial state. Motor off.
IDLE -> START_OPENING: 
     1. Open Button is pressed.
     2. Button is debounced(with delay)
IDLE -> START_CLOSING:
     1. Close Buttton is pressed.
     2. Button is debounced(with delay)
START_OPENING -> OPENING:
     1. Start motor.
START_CLOSING -> CLOSING:
     1. Start motor.
OPENING->IDLE:
     One of the following occurred.
     i) Open Button pressed again
     ii) Open Limit switch hit
         - backoff from limit switch
CLOSING->IDLE:
     One of the following occurred.
     i) Close Button pressed again
     ii) Close Limit switch hit 
         - backoff from limit switch
*/
enum ProgramState { IDLE,
                    START_OPENING,
                    OPENING,
                    START_CLOSING,
                    CLOSING };
// Initial state of program is IDLE
ProgramState programState = IDLE;



enum Limit { OPEN_LIMIT,
             CLOSE_LIMIT };
void backOffLimit(Limit limit);  // forward declaration(unfortunately necessary for Arduino)


//Delay for debouncing buttons
const int debounceDelay = 200;

//Time given for reversing motor to backoff from limit switches
const int backoffTimeMillis = 50;

// Delay for changing direction of motor when backing off
const int changeDirectionDelay = 50;

// Delay so that Sandy can press and hold the button, not quick press
const int pressAndHoldDelay = 2000;



void setup() {
  // Set motor B control pins to outputs

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);  // IN3 direction pin for Motor B
  pinMode(in4, OUTPUT);  // IN4 direction pin for Motor B

  Serial.begin(9600);
  Serial.println(F("Compiled " __DATE__ " " __TIME__));
  Serial.println(F("File: " __FILE__));
}

// Start Motor B clockwise: IN3 HIGH, IN4 LOW
void start_motor_opening() {
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enB, 255);
}

// Start Motor B anticlockwise: IN3 LOW, IN4 HIGH
void start_motor_closing() {
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enB, 255);
}

void stopMotor() {
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  //analogWrite(enB, 0);
}

void backOffLimit(Limit limit) {
  //stop motor
  stopMotor();
  delay(changeDirectionDelay);

  //open limit hit
  if (limit == OPEN_LIMIT) {
    start_motor_closing();
  }

  //close limit hit
  else {
    start_motor_opening();
  }

  delay(backoffTimeMillis);
  stopMotor();
}

int readPinValue(int pin) {
  int value = digitalRead(pin);
  return value;
}

void idle_state() {
  if (readPinValue(openButtonPin) == HIGH && readPinValue(openLimitPin) == LOW) {
    delay(debounceDelay);
    programState = START_OPENING;
  }

  if (readPinValue(closeButtonPin) == HIGH && readPinValue(closeLimitPin) == LOW) {
    delay(debounceDelay);
    programState = START_CLOSING;
  }
}

void start_opening() {
  start_motor_opening();
  programState = OPENING;

  delay(pressAndHoldDelay);
}

void start_closing() {
  start_motor_closing();
  programState = CLOSING;
}

void opening_state() {
  if (readPinValue(openLimitPin) == HIGH) {
    backOffLimit(OPEN_LIMIT);
    delay(debounceDelay);
    programState = IDLE;
  }

  if (readPinValue(openButtonPin) == HIGH) {
    stopMotor();
    delay(debounceDelay);
    programState = IDLE;
  }
}

void closing_state() {
  if (readPinValue(closeLimitPin) == HIGH) {
    stopMotor();
    backOffLimit(CLOSE_LIMIT);
    delay(debounceDelay);
    programState = IDLE;
  }

  if (readPinValue(closeButtonPin) == HIGH) {
    stopMotor();
    delay(debounceDelay);
    programState = IDLE;
  }
}

void loop() {
  switch (programState) {
    case IDLE:
      
      idle_state();
      break;

    case START_OPENING:
      //Serial.println("start opening state");
      start_opening();
      break;

    case START_CLOSING:
      start_closing();
      break;

    case OPENING:
      //Serial.println("curtain opening state");
      opening_state();
      break;

    case CLOSING:
      closing_state();
      break;
  }
}
