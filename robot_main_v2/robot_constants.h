#ifndef ROBOT_CONSTANTS_H
#define ROBOT_CONSTANTS_H

#include <stdint.h>

namespace RobotConst
{
    // ─── Motor PWM config ────────────────────────────────────────
    constexpr int PWM_FREQ = 1000;
    constexpr int PWM_CHANNEL_A = 0;
    constexpr int PWM_CHANNEL_B = 1;
    constexpr int PWM_RESOLUTION = 8;
    constexpr bool MOTOR_A_INVERTED = true;
    constexpr bool MOTOR_B_INVERTED = true;

    // ─── Autonomous drive ─────────────────────────────────────────
    constexpr uint8_t FORWARD_SPEED = 185;
    constexpr uint8_t TURN_SPEED = 165;
    constexpr unsigned long DURATION_FORWARD_1 = 1800;
    constexpr unsigned long DURATION_TURN_RIGHT = 700;
    constexpr unsigned long DURATION_FORWARD_2 = 1500;
    constexpr unsigned long DURATION_TURN_LEFT = 700;
    constexpr unsigned long DURATION_STOP = 700;

    // ─── Display palette/layout ───────────────────────────────────
    constexpr uint16_t BLACK = 0x0000;
    constexpr uint16_t WALLE_GREEN = 0x9E66;
    constexpr uint16_t WALLE_DIM = 0x5B40;
    constexpr uint16_t WALLE_DARK = 0x3240;
    constexpr uint16_t WALLE_BRIGHT = 0xCF28;

    constexpr int NUM_BARS = 12;
    constexpr int SUN_CX = 22;
    constexpr int SUN_CY = 55;
    constexpr int BAR_AREA_X = 44;
    constexpr int BAR_AREA_Y = 32;
    constexpr int BAR_W = 76;
    constexpr int BAR_H = 8;
    constexpr int BAR_GAP = 2;
    constexpr int COL_SEP_X = 40;
    constexpr unsigned long CHARGE_INTERVAL = 600;

    // ─── Servo IOC ────────────────────────────────────────────────
    constexpr int SERVOMIN = 150;
    constexpr int SERVOMAX = 600;
    constexpr int SERVO_FREQ = 50;
    constexpr unsigned long SERVO_STEP_DELAY_MS = 8;
    constexpr unsigned long POSE_DELAY_MS = 1500;
    constexpr int HEAD_CENTER_ANGLE = 90;

    constexpr uint8_t LEFT_ARM_SERVO_ID = 1;
    constexpr uint8_t RIGHT_ARM_SERVO_ID = 2;
    constexpr uint8_t HEAD_SERVO_ID = 3;
    constexpr int SERVO_CONTROLLER_CHANNELS = 16;
    constexpr int ARM_ANGLE_MIN = 0;
    constexpr int ARM_ANGLE_MAX = 120;
}

namespace RobotPins
{
    // ─── Motor pins ───────────────────────────────────────────────
    constexpr uint8_t ENA_PIN = 14;
    constexpr uint8_t IN1_PIN = 27;
    constexpr uint8_t IN2_PIN = 26;
    constexpr uint8_t IN3_PIN = 25;
    constexpr uint8_t IN4_PIN = 33;
    constexpr uint8_t ENB_PIN = 32;

    // ─── TFT pins ─────────────────────────────────────────────────
    constexpr uint8_t TFT_CS = 5;
    constexpr uint8_t TFT_RST = 15;
    constexpr uint8_t TFT_DC = 2;
    constexpr uint8_t TFT_MOSI = 13;
    constexpr uint8_t TFT_SCLK = 18;

    // ─── I2C pins ─────────────────────────────────────────────────
    // PCA9685 I2C reference: SDA = GPIO 21, SCL/SDL = GPIO 22
    constexpr uint8_t SERVO_I2C_SDA_PIN = 21;
    constexpr uint8_t SERVO_I2C_SCL_PIN = 22;
}

#endif