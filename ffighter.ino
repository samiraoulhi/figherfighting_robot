/*
  Robot anti-incendie - Arduino Uno + L298N + relais + servo
  2 capteurs de flamme (gauche / droite), machine à 4 états :
  SEARCH -> APPROACH -> EXTINGUISH -> COOLDOWN_ST
*/

#include <Servo.h>

// ====================== BROCHES ======================
const int leftSensor  = 12;   // capteur de flamme gauche
const int rightSensor = 13;   // capteur de flamme droit

const int ENA = 3, IN1 = 2,  IN2 = 4;     // moteur(s) gauche (ENA = PWM)
const int ENB = 5, IN3 = A0, IN4 = A1;    // moteur(s) droit  (ENB = PWM)

const int relayPin = 10;      // module relais -> pompe
const int servoPin = 9;       // servo de la lance

// ====================== RÉGLAGES ======================
const bool FLAME_DETECTED   = HIGH;   // tes capteurs sortent HIGH sur une flamme
const bool RELAY_ACTIVE_LOW = true;   // mettre false si la pompe démarre à l'envers

const int SPEED = 200;                // vitesse PWM (0-255)

const int SWEEP_MIN = 60, SWEEP_MAX = 120;          // angles du balayage servo
const unsigned long SWEEP_INTERVAL = 15;            // ms entre deux pas du servo

const unsigned long MAX_SPRAY    = 8000;   // arrosage max d'affilée (ms)
const unsigned long COOLDOWN_MS  = 5000;   // pause de sécurité après arrosage max
const unsigned long APPROACH_MS  = 1500;   // durée d'avance vers la flamme
const unsigned long SEARCH_TURN  = 150;    // durée d'un pas de rotation (mode recherche)
const unsigned long SEARCH_PAUSE = 500;    // pause pour "regarder"

// ====================== VARIABLES ======================
enum State { SEARCH, APPROACH, EXTINGUISH, COOLDOWN_ST };
State state = SEARCH;

Servo myservo;
int servoAngle = 90, servoStep = 2;

unsigned long lastSweep = 0, sprayStart = 0, cooldownStart = 0, driveStart = 0;
bool driving = false;
int  lastDir = 1;            // dernier côté où une flamme a été vue (-1 gauche, 1 droite)

// ====================== FONCTIONS ======================

// Lecture filtrée : 5 lectures, il en faut 4 positives (anti-parasites)
bool readFlame(int pin) {
  int count = 0;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(pin) == FLAME_DETECTED) count++;
    delay(1);
  }
  return count >= 4;
}

void setPump(bool on) {
  digitalWrite(relayPin, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

// dir : 1 = avant, -1 = arrière, 0 = arrêt
void setMotor(int in1, int in2, int en, int dir) {
  digitalWrite(in1, dir > 0);
  digitalWrite(in2, dir < 0);
  analogWrite(en, dir == 0 ? 0 : SPEED);
}

void stopRobot() { setMotor(IN1, IN2, ENA, 0);  setMotor(IN3, IN4, ENB, 0); }
void forward()   { setMotor(IN1, IN2, ENA, 1);  setMotor(IN3, IN4, ENB, 1); }
void turnLeft()  { setMotor(IN1, IN2, ENA, -1); setMotor(IN3, IN4, ENB, 1); }
void turnRight() { setMotor(IN1, IN2, ENA, 1);  setMotor(IN3, IN4, ENB, -1); }

// Balayage non bloquant de la lance
void sweepServo() {
  if (millis() - lastSweep >= SWEEP_INTERVAL) {
    lastSweep = millis();
    servoAngle += servoStep;
    if (servoAngle >= SWEEP_MAX || servoAngle <= SWEEP_MIN) servoStep = -servoStep;
    myservo.write(servoAngle);
  }
}

// Mode recherche : un pas de rotation, une pause, et on recommence.
// Le robot tourne du côté où il a vu la flamme en dernier.
void searchStep(unsigned long now) {
  static bool turning = true;
  static unsigned long t0 = 0;
  unsigned long dur = turning ? SEARCH_TURN : SEARCH_PAUSE;
  if (now - t0 >= dur) { turning = !turning; t0 = now; }

  if (turning) {
    if (lastDir < 0) turnLeft(); else turnRight();
  } else {
    stopRobot();
  }
}

// ====================== SETUP ======================
void setup() {
  pinMode(leftSensor, INPUT);
  pinMode(rightSensor, INPUT);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(relayPin, OUTPUT);
  setPump(false);                 // pompe éteinte dès le démarrage

  myservo.attach(servoPin);
  myservo.write(90);
  stopRobot();

  delay(2000);                    // temps de poser le robot avant qu'il bouge
}

// ====================== BOUCLE PRINCIPALE ======================
void loop() {
  // 1. Lecture des capteurs
  bool left  = readFlame(leftSensor);
  bool right = readFlame(rightSensor);
  bool fire  = left || right;     // au moins un capteur voit une flamme
  bool front = left && right;     // les deux voient = flamme en face
  unsigned long now = millis();

  // 2. On mémorise le côté de la flamme
  if (left && !right) lastDir = -1;
  else if (right && !left) lastDir = 1;

  // 3. Machine à états
  switch (state) {

    case SEARCH:                              // je cherche
      setPump(false);
      myservo.write(90);
      if (fire) { stopRobot(); driving = false; state = APPROACH; break; }
      searchStep(now);
      break;

    case APPROACH:                            // je m'oriente puis j'avance
      setPump(false);
      myservo.write(90);
      if (!fire) { driving = false; stopRobot(); state = SEARCH; break; }

      if (front) {
        if (!driving) { driving = true; driveStart = now; }
        forward();
        if (now - driveStart >= APPROACH_MS) {
          driving = false;
          stopRobot();
          sprayStart = now;
          state = EXTINGUISH;
        }
      } else {
        driving = false;
        if (left) turnLeft(); else turnRight();
      }
      break;

    case EXTINGUISH:                          // j'arrose
      stopRobot();
      if (!fire) {                            // flamme éteinte
        setPump(false);
        myservo.write(90);
        state = SEARCH;
        break;
      }
      setPump(true);
      sweepServo();
      if (now - sprayStart >= MAX_SPRAY) {    // sécurité : arrosage trop long
        setPump(false);
        myservo.write(90);
        cooldownStart = now;
        state = COOLDOWN_ST;
      }
      break;

    case COOLDOWN_ST:                         // pause de sécurité
      stopRobot();
      setPump(false);
      if (now - cooldownStart >= COOLDOWN_MS) state = fire ? APPROACH : SEARCH;
      break;
  }
}
