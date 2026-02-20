#ifndef SERVO_IOC_MODULE_H
#define SERVO_IOC_MODULE_H

void initServoIOC();
void updateServoIOC();
void setServoAutoPoseEnabled(bool enabled);
bool isServoAutoPoseEnabled();
void setHeadServoAngle(int angle);
void setLeftArmServoAngle(int angle);
void setRightArmServoAngle(int angle);

#endif
