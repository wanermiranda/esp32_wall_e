#include "autonomous_drive.h"
#include "motor_control.h"
#include "robot_constants.h"

enum DriveState
{
    DRIVE_FORWARD_1,
    TURN_RIGHT,
    DRIVE_FORWARD_2,
    TURN_LEFT,
    DRIVE_STOP
};

namespace
{
    DriveState driveState = DRIVE_FORWARD_1;
    unsigned long stateStartMs = 0;

    void setDriveState(DriveState nextState)
    {
        driveState = nextState;
        stateStartMs = millis();

        switch (driveState)
        {
        case DRIVE_FORWARD_1:
            driveTank(RobotConst::FORWARD_SPEED, RobotConst::FORWARD_SPEED);
            Serial.println("Drive: FORWARD 1");
            break;
        case TURN_RIGHT:
            driveTank(RobotConst::TURN_SPEED, -RobotConst::TURN_SPEED);
            Serial.println("Drive: TURN RIGHT");
            break;
        case DRIVE_FORWARD_2:
            driveTank(RobotConst::FORWARD_SPEED, RobotConst::FORWARD_SPEED);
            Serial.println("Drive: FORWARD 2");
            break;
        case TURN_LEFT:
            driveTank(-RobotConst::TURN_SPEED, RobotConst::TURN_SPEED);
            Serial.println("Drive: TURN LEFT");
            break;
        case DRIVE_STOP:
            stopMotors();
            Serial.println("Drive: STOP");
            break;
        }
    }
}

void initAutonomousDrive()
{
    setDriveState(DRIVE_FORWARD_1);
}

void updateAutonomousDrive()
{
    unsigned long now = millis();
    unsigned long elapsed = now - stateStartMs;

    switch (driveState)
    {
    case DRIVE_FORWARD_1:
        if (elapsed >= RobotConst::DURATION_FORWARD_1)
            setDriveState(TURN_RIGHT);
        break;
    case TURN_RIGHT:
        if (elapsed >= RobotConst::DURATION_TURN_RIGHT)
            setDriveState(DRIVE_FORWARD_2);
        break;
    case DRIVE_FORWARD_2:
        if (elapsed >= RobotConst::DURATION_FORWARD_2)
            setDriveState(TURN_LEFT);
        break;
    case TURN_LEFT:
        if (elapsed >= RobotConst::DURATION_TURN_LEFT)
            setDriveState(DRIVE_STOP);
        break;
    case DRIVE_STOP:
        if (elapsed >= RobotConst::DURATION_STOP)
            setDriveState(DRIVE_FORWARD_1);
        break;
    }
}
