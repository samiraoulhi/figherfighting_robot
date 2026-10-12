#include <Servo.h>

// ================= MOTOR PINS =================
#define IN1 10
#define IN2 9
#define IN3 8
#define IN4 7

#define ENA 11
#define ENB 6

// ================= FLAME SENSORS =================
#define FLAME_LEFT   A0
#define FLAME_CENTER A1
#define FLAME_RIGHT  A2

// ================= SERVO & RELAY =================
#define SERVO_PIN 5
#define RELAY_PIN 4

// ================= SETTINGS =================

// Your sensor readings:
// No fire       = around 915
// Fire far      = around 800
// Fire close    = around 300-400
// Stop point    = 600

#define FIRE_DETECT 850
#define FIRE_STOP   600

// Motor speed
int speedLeft = 140;
int speedRight = 140;

// Short movement time
int moveTime = 200;

// Servo positions
int servoLeft   = 50;
int servoCenter = 90;
int servoRight  = 130;

// Pump spray time at each servo position
int sprayDelay = 700;

// Change this if your relay works opposite
// HIGH = relay ON
// LOW  = relay OFF
#define RELAY_ON  HIGH
#define RELAY_OFF LOW

Servo fireServo;


// ================= SETUP =================

void setup() {

  Serial.begin(9600);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  // Servo
  fireServo.attach(SERVO_PIN);
  fireServo.write(servoCenter);

  stopCar();

  delay(2000);

  Serial.println("Fire Fighter Robot Ready");
}


// ================= MAIN LOOP =================

void loop() {

  // Read flame sensors
  int leftValue   = analogRead(FLAME_LEFT);
  int centerValue = analogRead(FLAME_CENTER);
  int rightValue  = analogRead(FLAME_RIGHT);

  // Show values
  Serial.print("Left: ");
  Serial.print(leftValue);

  Serial.print("   Center: ");
  Serial.print(centerValue);

  Serial.print("   Right: ");
  Serial.println(rightValue);


  // ==========================================
  // NO FIRE
  // ==========================================

  if (leftValue > FIRE_DETECT &&
      centerValue > FIRE_DETECT &&
      rightValue > FIRE_DETECT) {

    stopCar();
    pumpOff();

    delay(100);

    return;
  }


  // ==========================================
  // FIRE IS CLOSE
  // 600 OR LOWER = STOP
  // ==========================================

  if (leftValue <= FIRE_STOP ||
      centerValue <= FIRE_STOP ||
      rightValue <= FIRE_STOP) {

    stopCar();

    Serial.println("FIRE CLOSE - STOPPING");

    sprayWater();

    delay(300);

    return;
  }


  // ==========================================
  // FIRE DETECTED - FIND DIRECTION
  // LOWER VALUE = STRONGER FLAME
  // ==========================================

  // LEFT is strongest
  if (leftValue < centerValue &&
      leftValue < rightValue &&
      leftValue < FIRE_DETECT) {

    Serial.println("Fire LEFT");

    turnLeft();

    delay(moveTime);

    stopCar();

    delay(150);

    return;
  }


  // RIGHT is strongest
  if (rightValue < leftValue &&
      rightValue < centerValue &&
      rightValue < FIRE_DETECT) {

    Serial.println("Fire RIGHT");

    turnRight();

    delay(moveTime);

    stopCar();

    delay(150);

    return;
  }


  // CENTER is strongest
  if (centerValue < leftValue &&
      centerValue < rightValue &&
      centerValue < FIRE_DETECT) {

    Serial.println("Fire CENTER");

    forward();

    delay(moveTime);

    stopCar();

    delay(150);

    return;
  }


  // Safety
  stopCar();

  delay(100);
}


// =================================================
// MOTOR FUNCTIONS
// =================================================

void forward() {

  analogWrite(ENA, speedRight);
  analogWrite(ENB, speedLeft);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


void backward() {

  analogWrite(ENA, speedRight);
  analogWrite(ENB, speedLeft);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}


void turnRight() {

  analogWrite(ENA, speedRight);
  analogWrite(ENB, speedLeft);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


void turnLeft() {

  analogWrite(ENA, speedRight);
  analogWrite(ENB, speedLeft);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}


void stopCar() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// =================================================
// PUMP / RELAY
// =================================================

void pumpOn() {

  digitalWrite(RELAY_PIN, RELAY_ON);

  Serial.println("PUMP ON");
}


void pumpOff() {

  digitalWrite(RELAY_PIN, RELAY_OFF);

  Serial.println("PUMP OFF");
}


// =================================================
// WATER SPRAY
// =================================================

void sprayWater() {

  Serial.println("START SPRAYING");

  stopCar();

  // Pump ON
  pumpOn();

  // Quick left-right sweep
  for (int i = 0; i < 3; i++) {

    fireServo.write(servoLeft);
    delay(150);

    fireServo.write(servoRight);
    delay(150);
  }

  // Return to center
  fireServo.write(servoCenter);
  delay(100);

  // Pump OFF
  pumpOff();

  Serial.println("SPRAY COMPLETE");

  delay(500);
}
