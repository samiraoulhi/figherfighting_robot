#include <Wire.h>
#include <Servo.h>

// Define the pins for the flame sensors, motors, and servo
const int leftSensor = 9;
const int rightSensor = 10;
const int forwardSensor = 8;
const int leftMotor1 = 2;
const int leftMotor2 = 3;
const int rightMotor1 = 4;
const int rightMotor2 = 5;
const int waterPump = 6;
const int servoPin = 7; // Adjust the pin number if necessary

// Create a servo object
Servo myservo;

// Declare a boolean variable to track water pump status and fire status
bool isWaterPumpOn = false;
bool fire = false;

// Initialize the flame sensors, motors, and servo
void setup()
{
    pinMode(leftSensor, INPUT);
    pinMode(rightSensor, INPUT);
    pinMode(forwardSensor, INPUT);
    pinMode(leftMotor1, OUTPUT);
    pinMode(leftMotor2, OUTPUT);
    pinMode(rightMotor1, OUTPUT);
    pinMode(rightMotor2, OUTPUT);
    pinMode(waterPump, OUTPUT); // Set the water pump pin as an output

    // Attach the servo to the pin
    myservo.attach(servoPin);

    // Set the initial position of the servo
    myservo.write(0);
}

// Loop forever
void loop()
{
    // Check if there is fire
    if (digitalRead(leftSensor) == HIGH || digitalRead(rightSensor) == HIGH || digitalRead(forwardSensor) == HIGH)
    {
        fire = true;
    }
    else
    {
        fire = false;
    }

    // If there is fire, move the robot towards it
    if (fire)
    {
        if (digitalRead(leftSensor) == HIGH)
        {
            // Turn left
            digitalWrite(leftMotor1, HIGH);
            digitalWrite(leftMotor2, LOW);
            digitalWrite(rightMotor1, LOW);
            digitalWrite(rightMotor2, HIGH);
        }
        else if (digitalRead(rightSensor) == HIGH)
        {
            // Turn right
            digitalWrite(leftMotor1, LOW);
            digitalWrite(leftMotor2, HIGH);
            digitalWrite(rightMotor1, HIGH);
            digitalWrite(rightMotor2, LOW);
        }
        else if (digitalRead(forwardSensor) == HIGH)
        {
            // Move forward
            digitalWrite(leftMotor1, HIGH);
            digitalWrite(leftMotor2, LOW);
            digitalWrite(rightMotor1, HIGH);
            digitalWrite(rightMotor2, LOW);
        }
    }
    else
    {
        // No fire detected, stop the motors
        digitalWrite(leftMotor1, LOW);
        digitalWrite(leftMotor2, LOW);
        digitalWrite(rightMotor1, LOW);
        digitalWrite(rightMotor2, LOW);
    }

    // If the robot is close to the fire, turn on the water pump and spray water
    if (fire && (digitalRead(leftSensor) == HIGH || digitalRead(rightSensor) == HIGH || digitalRead(forwardSensor) == HIGH))
    {
        digitalWrite(waterPump, HIGH);
        isWaterPumpOn = true;

        // Rotate the servo to spray water at the sight where the fire is detected
        myservo.write(120);
        delay(500); // Delay for spraying water
    }
    else
    {
        digitalWrite(waterPump, LOW);
        isWaterPumpOn = false;

        // Set the servo back to 0 degrees when the fire is extinguished
        myservo.write(0);
    }
}
