#include <ESP32Servo.h>
#define SERVO_PIN 26  // ESP32 pin GPIO26 connected to servo motor
Servo servoMotor;

const int pins[] = { 18, 25 };
int servoPos = 0;
int pin_count = 2;


void moveServo(int count, int stepDir){
  for (int i = 0; i < count; i ++) {
    servoPos += stepDir;
    servoMotor.write(servoPos);
    Serial.print("Servo Pos ");
    Serial.print(servoPos);
    Serial.print(" Dir ");
    Serial.println(stepDir);
    delay(10); // waits 10ms to reach the position
  }
}

void setup() {
  Serial.begin(9600);
  Serial.print("Attaching Servo Motor");
  servoMotor.attach(SERVO_PIN);  // attaches the servo on ESP32 pin
  moveServo(0, 1);
  delay(100);
  for (int i = 0; i < pin_count; i++) {
    Serial.print("Enable output pin: ");
    Serial.println(pins[i]);
    pinMode(pins[i], OUTPUT);
  }
}

void loop() {
  int dir = 1;
  for (int i = 0; i < pin_count; i++) {
    Serial.print("Setting HIGH pin: ");
    Serial.println(pins[i]);
    digitalWrite(pins[i], HIGH);  // Set GPIOxx active high
    moveServo(180, dir);
    digitalWrite(pins[i], LOW);  // Set GPIOxx active high
    dir = dir * -1;
  }
}
