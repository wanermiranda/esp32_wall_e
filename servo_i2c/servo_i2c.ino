#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Create a PCA9685 object
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Define the servo parameters
#define SERVOMIN 150  // Minimum pulse length count
#define SERVOMAX 600  // Maximum pulse length count
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz
#define SERVO_STEP_DELAY_MS 8
#define POSE_DELAY_MS 1500
#define HEAD_CENTER_ANGLE 90

const uint8_t LEFT_ARM_SERVO_ID = 1;
const uint8_t RIGHT_ARM_SERVO_ID = 2;
const uint8_t HEAD_SERVO_ID = 3;

int lastAngleByChannel[16];

bool isHeadCentered()
{
    int headAngle = lastAngleByChannel[HEAD_SERVO_ID - 1];
    return headAngle == HEAD_CENTER_ANGLE;
}

bool moveServoById(uint8_t servoId, int angle)
{
    if (servoId < 1 || servoId > 16)
    {
        Serial.print("[ERROR] Invalid servo ID: ");
        Serial.println(servoId);
        return false;
    }

    int safeAngle = constrain(angle, 0, 180);
    if (safeAngle != angle)
    {
        Serial.print("[WARN] Angle out of range, clamped from ");
        Serial.print(angle);
        Serial.print(" to ");
        Serial.println(safeAngle);
    }

    uint8_t channel = servoId - 1;
    int startAngle = lastAngleByChannel[channel];

    if (startAngle < 0)
    {
        int pulse = map(safeAngle, 0, 180, SERVOMIN, SERVOMAX);
        pwm.setPWM(channel, 0, pulse);
        lastAngleByChannel[channel] = safeAngle;

        Serial.print("[DEBUG] Servo ");
        Serial.print(servoId);
        Serial.print(" first move -> ");
        Serial.print(safeAngle);
        Serial.print(" deg (pulse ");
        Serial.print(pulse);
        Serial.println(")");
        return true;
    }

    int stepDirection = (safeAngle >= startAngle) ? 1 : -1;
    for (int currentAngle = startAngle; currentAngle != safeAngle; currentAngle += stepDirection)
    {
        int pulse = map(currentAngle, 0, 180, SERVOMIN, SERVOMAX);
        pwm.setPWM(channel, 0, pulse);
        delay(SERVO_STEP_DELAY_MS);
    }

    int finalPulse = map(safeAngle, 0, 180, SERVOMIN, SERVOMAX);
    pwm.setPWM(channel, 0, finalPulse);
    lastAngleByChannel[channel] = safeAngle;

    Serial.print("[DEBUG] Servo ");
    Serial.print(servoId);
    Serial.print(" moved incrementally to ");
    Serial.print(safeAngle);
    Serial.print(" deg (pulse ");
    Serial.print(finalPulse);
    Serial.println(")");

    return true;
}

void moveLeftArm(int angle)
{
    if (!isHeadCentered())
    {
        Serial.println("[WARN] Left arm blocked: head is not centered");
        return;
    }

    int safeArmAngle = constrain(angle, 0, 120);
    int invertedAngle = 180 - safeArmAngle;
    moveServoById(LEFT_ARM_SERVO_ID, invertedAngle);
}

void moveRightArm(int angle)
{
    if (!isHeadCentered())
    {
        Serial.println("[WARN] Right arm blocked: head is not centered");
        return;
    }

    int safeArmAngle = constrain(angle, 0, 120);
    moveServoById(RIGHT_ARM_SERVO_ID, safeArmAngle);
}

void moveHead(int angle)
{
    moveServoById(HEAD_SERVO_ID, angle);
}

void setup()
{
    for (int i = 0; i < 16; i++)
    {
        lastAngleByChannel[i] = -1;
    }

    Serial.begin(115200);
    delay(200);
    Serial.println("[INFO] Servo IOC starting...");

    pwm.begin();
    pwm.setPWMFreq(SERVO_FREQ); // Set PWM frequency to 50 Hz
    Serial.println("[INFO] PCA9685 initialized");
    Serial.print("[INFO] Servo range: ");
    Serial.print(SERVOMIN);
    Serial.print(" to ");
    Serial.println(SERVOMAX);
    centerServos();
}

void centerServos()
{
    moveHead(HEAD_CENTER_ANGLE);
    delay(POSE_DELAY_MS);
    moveLeftArm(60);
    moveRightArm(60);
}

void loop()
{
    centerServos();

    delay(POSE_DELAY_MS);

    moveLeftArm(120);
    moveRightArm(120);

    delay(POSE_DELAY_MS);

    centerServos();

    delay(POSE_DELAY_MS);

    moveLeftArm(0);
    moveRightArm(0);

    delay(POSE_DELAY_MS);

    moveHead(0);

    delay(POSE_DELAY_MS);

    moveHead(180);

    delay(POSE_DELAY_MS);
}
