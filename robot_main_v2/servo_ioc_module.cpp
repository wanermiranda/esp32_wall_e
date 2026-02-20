#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "servo_ioc_module.h"
#include "robot_constants.h"

namespace
{
    Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
    bool autoPoseEnabled = true;

    int currentAngleByChannel[RobotConst::SERVO_CONTROLLER_CHANNELS];
    int targetAngleByChannel[RobotConst::SERVO_CONTROLLER_CHANNELS];
    unsigned long lastStepMsByChannel[RobotConst::SERVO_CONTROLLER_CHANNELS];

    enum ServoPose
    {
        POSE_CENTER,
        POSE_ARMS_UP,
        POSE_CENTER_AGAIN,
        POSE_ARMS_DOWN,
        POSE_HEAD_LEFT,
        POSE_HEAD_RIGHT
    };

    ServoPose pose = POSE_CENTER;
    unsigned long poseStartMs = 0;
    bool poseApplied = false;

    uint8_t toChannel(uint8_t servoId)
    {
        return servoId - 1;
    }

    bool isValidServoId(uint8_t servoId)
    {
        return servoId >= 1 && servoId <= RobotConst::SERVO_CONTROLLER_CHANNELS;
    }

    int readCurrentAngle(uint8_t servoId)
    {
        return currentAngleByChannel[toChannel(servoId)];
    }

    bool isHeadCentered()
    {
        return readCurrentAngle(RobotConst::HEAD_SERVO_ID) == RobotConst::HEAD_CENTER_ANGLE;
    }

    void writeServoAngleImmediate(uint8_t servoId, int angle)
    {
        uint8_t channel = toChannel(servoId);
        int safeAngle = constrain(angle, 0, 180);
        int pulse = map(safeAngle, 0, 180, RobotConst::SERVOMIN, RobotConst::SERVOMAX);
        pwm.setPWM(channel, 0, pulse);
        currentAngleByChannel[channel] = safeAngle;
        targetAngleByChannel[channel] = safeAngle;
        lastStepMsByChannel[channel] = millis();
    }

    bool setServoTargetById(uint8_t servoId, int angle)
    {
        if (!isValidServoId(servoId))
        {
            Serial.print("[ERROR] Invalid servo ID: ");
            Serial.println(servoId);
            return false;
        }

        int safeAngle = constrain(angle, 0, 180);
        uint8_t channel = toChannel(servoId);
        targetAngleByChannel[channel] = safeAngle;

        if (currentAngleByChannel[channel] < 0)
        {
            writeServoAngleImmediate(servoId, safeAngle);
        }

        return true;
    }

    void setHeadTarget(int angle)
    {
        setServoTargetById(RobotConst::HEAD_SERVO_ID, angle);
    }

    void setLeftArmTarget(int angle)
    {
        int safeArmAngle = constrain(angle, RobotConst::ARM_ANGLE_MIN, RobotConst::ARM_ANGLE_MAX);
        int invertedAngle = 180 - safeArmAngle;
        setServoTargetById(RobotConst::LEFT_ARM_SERVO_ID, invertedAngle);
    }

    void setRightArmTarget(int angle)
    {
        int safeArmAngle = constrain(angle, RobotConst::ARM_ANGLE_MIN, RobotConst::ARM_ANGLE_MAX);
        setServoTargetById(RobotConst::RIGHT_ARM_SERVO_ID, safeArmAngle);
    }

    void applyPose(ServoPose nextPose)
    {
        switch (nextPose)
        {
        case POSE_CENTER:
            setHeadTarget(RobotConst::HEAD_CENTER_ANGLE);
            setLeftArmTarget(60);
            setRightArmTarget(60);
            break;
        case POSE_ARMS_UP:
            setLeftArmTarget(120);
            setRightArmTarget(120);
            break;
        case POSE_CENTER_AGAIN:
            setHeadTarget(RobotConst::HEAD_CENTER_ANGLE);
            setLeftArmTarget(60);
            setRightArmTarget(60);
            break;
        case POSE_ARMS_DOWN:
            setLeftArmTarget(0);
            setRightArmTarget(0);
            break;
        case POSE_HEAD_LEFT:
            setHeadTarget(0);
            break;
        case POSE_HEAD_RIGHT:
            setHeadTarget(180);
            break;
        }
    }

    ServoPose nextPose(ServoPose current)
    {
        switch (current)
        {
        case POSE_CENTER:
            return POSE_ARMS_UP;
        case POSE_ARMS_UP:
            return POSE_CENTER_AGAIN;
        case POSE_CENTER_AGAIN:
            return POSE_ARMS_DOWN;
        case POSE_ARMS_DOWN:
            return POSE_HEAD_LEFT;
        case POSE_HEAD_LEFT:
            return POSE_HEAD_RIGHT;
        case POSE_HEAD_RIGHT:
        default:
            return POSE_CENTER;
        }
    }

    void updateOneServoStep(uint8_t servoId)
    {
        uint8_t channel = toChannel(servoId);
        int current = currentAngleByChannel[channel];
        int target = targetAngleByChannel[channel];

        if (current < 0 || current == target)
            return;

        unsigned long now = millis();
        if (now - lastStepMsByChannel[channel] < RobotConst::SERVO_STEP_DELAY_MS)
            return;

        int direction = (target > current) ? 1 : -1;
        int next = current + direction;
        int pulse = map(next, 0, 180, RobotConst::SERVOMIN, RobotConst::SERVOMAX);
        pwm.setPWM(channel, 0, pulse);
        currentAngleByChannel[channel] = next;
        lastStepMsByChannel[channel] = now;
    }

    void updateAllServos()
    {
        updateOneServoStep(RobotConst::LEFT_ARM_SERVO_ID);
        updateOneServoStep(RobotConst::RIGHT_ARM_SERVO_ID);
        updateOneServoStep(RobotConst::HEAD_SERVO_ID);
    }
}

void initServoIOC()
{
    for (int i = 0; i < RobotConst::SERVO_CONTROLLER_CHANNELS; i++)
    {
        currentAngleByChannel[i] = -1;
        targetAngleByChannel[i] = -1;
        lastStepMsByChannel[i] = 0;
    }

    Wire.begin(RobotPins::SERVO_I2C_SDA_PIN, RobotPins::SERVO_I2C_SCL_PIN);

    pwm.begin();
    pwm.setPWMFreq(RobotConst::SERVO_FREQ);

    writeServoAngleImmediate(RobotConst::HEAD_SERVO_ID, RobotConst::HEAD_CENTER_ANGLE);
    writeServoAngleImmediate(RobotConst::LEFT_ARM_SERVO_ID, 180 - 60);
    writeServoAngleImmediate(RobotConst::RIGHT_ARM_SERVO_ID, 60);

    pose = POSE_CENTER;
    poseStartMs = millis();
    poseApplied = true;

    Serial.println("[INFO] Servo IOC module ready");
}

void updateServoIOC()
{
    updateAllServos();

    if (!autoPoseEnabled)
        return;

    unsigned long now = millis();
    if (now - poseStartMs < RobotConst::POSE_DELAY_MS)
        return;

    if (!poseApplied)
    {
        applyPose(pose);
        poseApplied = true;
        poseStartMs = now;
        return;
    }

    pose = nextPose(pose);
    poseApplied = false;
    poseStartMs = now;
}

void setServoAutoPoseEnabled(bool enabled)
{
    autoPoseEnabled = enabled;

    if (enabled)
    {
        pose = POSE_CENTER;
        poseApplied = false;
        poseStartMs = millis();
    }
}

bool isServoAutoPoseEnabled()
{
    return autoPoseEnabled;
}

void setHeadServoAngle(int angle)
{
    setHeadTarget(angle);
}

void setLeftArmServoAngle(int angle)
{
    setLeftArmTarget(angle);
}

void setRightArmServoAngle(int angle)
{
    setRightArmTarget(angle);
}
