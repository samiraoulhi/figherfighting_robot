#include <Servo.h>

// ---------- Broches ----------
const int leftSensor    = 12;
const int forwardSensor = 8;
const int rightSensor   = 13;

const int ENA = 3, IN1 = 2,  IN2 = 4;    // moteur gauche
const int ENB = 5, IN3 = A0, IN4 = A1;   // moteur droit

const int relayPin = 10;
const int servoPin = 9;

// ---------- Réglages ----------
const bool FLAME_DETECTED   = LOW;    // teste ton module : certains sortent HIGH
const bool RELAY_ACTIVE_LOW = true;   // la plupart des modules relais
const int  SPEED            = 180;    // 0-255
const int  SWEEP_MIN = 60, SWEEP_MAX = 120;
const unsigned long SWEEP_INTERVAL = 15;    // ms entre deux pas du servo

const unsigned long MAX_SPRAY    = 8000;    // arrosage max d'affilée (ms)
const unsigned long COOLDOWN_MS  = 5000;    // pause après un arrosage max (ms)
const unsigned long APPROACH_MS  = 1500;    // avance vers la flamme avant d'arroser (ms)
const unsigned long SEARCH_TURN  = 250;     // rotation en mode recherche (ms)
const unsigned long SEARCH_PAUSE = 400;     // arrêt pour "regarder" (ms)

enum State { SEARCH, APPROACH, EXTINGUISH, COOLDOWN_ST };
State state = SEARCH;

Servo myservo;
int servoAngle = 90, servoStep = 2;
unsigned long lastSweep = 0, sprayStart = 0, cooldownStart = 0, driveStart = 0;
bool driving = false;

// ---------- Utilitaires ----------
void setPump(bool on) {
  digitalWrite(relayPin, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void setMotor(int in1, int in2, int en, int dir) {  // 1 avant, -1 arrière, 0 arrêt
  digitalWrite(in1, dir > 0);
  digitalWrite(in2, dir < 0);
  analogWrite(en, dir == 0 ? 0 : SPEED);
}

void stopRobot()  { setMotor(IN1, IN2, ENA, 0);  setMotor(IN3, IN4, ENB, 0); }
void forward()    { setMotor(IN1, IN2, ENA, 1);  setMotor(IN3, IN4, ENB, 1); }
void turnLeft()   { setMotor(IN1, IN2, ENA, -1); setMotor(IN3, IN4, ENB, 1); }
void turnRight()  { setMotor(IN1, IN2, ENA, 1);  setMotor(IN3, IN4, ENB, -1); }

void sweepServo() {
  if (millis() - lastSweep >= SWEEP_INTERVAL) {
    lastSweep = millis();
    servoAngle += servoStep;
    if (servoAngle >= SWEEP_MAX || servoAngle <= SWEEP_MIN) servoStep = -servoStep;
    myservo.write(servoAngle);
  }
}

// Mode recherche : tourne un peu, s'arrête, regarde, recommence
void searchStep(unsigned long now) {
  static bool turning = true;
  static unsigned long t0 = 0;
  unsigned long dur = turning ? SEARCH_TURN : SEARCH_PAUSE;
  if (now - t0 >= dur) { turning = !turning; t0 = now; }
  if (turning) turnRight(); else stopRobot();
}

void setup() {
  pinMode(leftSensor, INPUT);
  pinMode(forwardSensor, INPUT);
  pinMode(rightSensor, INPUT);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(relayPin, OUTPUT);
  setPump(false);

  myservo.attach(servoPin);
  myservo.write(90);
  stopRobot();
}

void loop() {
  bool left  = digitalRead(leftSensor)    == FLAME_DETECTED;
  bool front = digitalRead(forwardSensor) == FLAME_DETECTED;
  bool right = digitalRead(rightSensor)   == FLAME_DETECTED;
  bool fire  = left || front || right;
  unsigned long now = millis();

  switch (state) {

    case SEARCH:
      setPump(false);
      myservo.write(90);
      if (fire) { stopRobot(); state = APPROACH; break; }
      searchStep(now);
      break;

    case APPROACH:
      setPump(false);
      myservo.write(90);
      if (!fire) { driving = false; stopRobot(); state = SEARCH; break; }

      if (front) {                       // flamme en face : on avance un peu
        if (!driving) { driving = true; driveStart = now; }
        forward();
        if (now - driveStart >= APPROACH_MS) {
          driving = false;
          stopRobot();
          sprayStart = now;
          state = EXTINGUISH;
        }
      } else {                           // sinon on s'oriente vers elle
        driving = false;
        if (left) turnLeft(); else turnRight();
      }
      break;

    case EXTINGUISH:
      stopRobot();
      if (!front) {                      // flamme éteinte ou perdue
        setPump(false);
        myservo.write(90);
        state = fire ? APPROACH : SEARCH;
        break;
      }
      setPump(true);
      sweepServo();
      if (now - sprayStart >= MAX_SPRAY) {   // sécurité : arrosage trop long
        setPump(false);
        myservo.write(90);
        cooldownStart = now;
        state = COOLDOWN_ST;
      }
      break;

    case COOLDOWN_ST:
      stopRobot();
      setPump(false);
      if (now - cooldownStart >= COOLDOWN_MS) state = fire ? APPROACH : SEARCH;
      break;
  }
}
