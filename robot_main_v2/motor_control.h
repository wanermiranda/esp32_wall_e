#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

void initMotors();
void setMotorA(bool fwd, uint8_t spd);
void setMotorB(bool fwd, uint8_t spd);
void stopMotors();
void driveTank(int leftSpeed, int rightSpeed);

#endif
