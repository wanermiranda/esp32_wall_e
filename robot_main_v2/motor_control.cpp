#include "motor_control.h"
#include "robot_constants.h"

void initMotors()
{
    pinMode(RobotPins::IN1_PIN, OUTPUT);
    pinMode(RobotPins::IN2_PIN, OUTPUT);
    pinMode(RobotPins::IN3_PIN, OUTPUT);
    pinMode(RobotPins::IN4_PIN, OUTPUT);

    ledcAttachChannel(RobotPins::ENA_PIN, RobotConst::PWM_FREQ, RobotConst::PWM_RESOLUTION, RobotConst::PWM_CHANNEL_A);
    ledcAttachChannel(RobotPins::ENB_PIN, RobotConst::PWM_FREQ, RobotConst::PWM_RESOLUTION, RobotConst::PWM_CHANNEL_B);
    stopMotors();
}

void setMotorA(bool fwd, uint8_t spd)
{
    digitalWrite(RobotPins::IN1_PIN, fwd ? HIGH : LOW);
    digitalWrite(RobotPins::IN2_PIN, fwd ? LOW : HIGH);
    ledcWrite(RobotPins::ENA_PIN, spd);
}

void setMotorB(bool fwd, uint8_t spd)
{
    digitalWrite(RobotPins::IN3_PIN, fwd ? HIGH : LOW);
    digitalWrite(RobotPins::IN4_PIN, fwd ? LOW : HIGH);
    ledcWrite(RobotPins::ENB_PIN, spd);
}

void stopMotors()
{
    digitalWrite(RobotPins::IN1_PIN, LOW);
    digitalWrite(RobotPins::IN2_PIN, LOW);
    digitalWrite(RobotPins::IN3_PIN, LOW);
    digitalWrite(RobotPins::IN4_PIN, LOW);
    ledcWrite(RobotPins::ENA_PIN, 0);
    ledcWrite(RobotPins::ENB_PIN, 0);
}

void driveTank(int leftSpeed, int rightSpeed)
{
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    if (leftSpeed == 0)
    {
        digitalWrite(RobotPins::IN1_PIN, LOW);
        digitalWrite(RobotPins::IN2_PIN, LOW);
        ledcWrite(RobotPins::ENA_PIN, 0);
    }
    else
    {
        setMotorA(leftSpeed > 0, (uint8_t)abs(leftSpeed));
    }

    if (rightSpeed == 0)
    {
        digitalWrite(RobotPins::IN3_PIN, LOW);
        digitalWrite(RobotPins::IN4_PIN, LOW);
        ledcWrite(RobotPins::ENB_PIN, 0);
    }
    else
    {
        setMotorB(rightSpeed > 0, (uint8_t)abs(rightSpeed));
    }
}
