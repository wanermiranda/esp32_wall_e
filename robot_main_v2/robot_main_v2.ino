/**
 * Robot Main v2 — ESP32 WROOM
 *
 * Combines:
 *   • DC motor control via L298N (autonomous sequence, no joystick)
 *   • Wall-E Solar Charge Level gauge on ST7735 TFT
 *
 * Modular layout:
 *   • motor_control.*
 *   • autonomous_drive.*
 *   • display_gauge.*
 */

#include "motor_control.h"
#include "autonomous_drive.h"
#include "display_gauge.h"
#include "servo_ioc_module.h"

// ═══════════════════════════════════════════════════════════════
//  Setup
// ═══════════════════════════════════════════════════════════════

void setup()
{
    Serial.begin(115200);
    Serial.println("== Robot Main v2 ==");

    initMotors();
    initGaugeDisplay();
    initAutonomousDrive();
    initServoIOC();

    Serial.println("Ready.");
}

// ═══════════════════════════════════════════════════════════════
//  Main loop
// ═══════════════════════════════════════════════════════════════

void loop()
{
    updateAutonomousDrive();
    updateCharge();
    updateServoIOC();
    delay(50);
}
