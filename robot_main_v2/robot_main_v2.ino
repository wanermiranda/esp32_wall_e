/**
 * Robot Main v2 — ESP32 WROOM
 *
 * Combines:
 *   • DC motor control via L298N (autonomous sequence, no joystick)
 *   • Wall-E Solar Charge Level gauge on ST7735 TFT
 *   • WiFi hotspot + web controls (motion/head/arms)
 *
 * Modular layout:
 *   • motor_control.*
 *   • autonomous_drive.*
 *   • display_gauge.*
 *   • wifi_ap.*
 *   • web_ui.*
 */

#include <WiFi.h>
#include <WebServer.h>

#include "motor_control.h"
#include "autonomous_drive.h"
#include "display_gauge.h"
#include "servo_ioc_module.h"
#include "wifi_ap.h"
#include "web_ui.h"

namespace
{
    constexpr uint16_t HTTP_PORT = 80;
    constexpr int DEFAULT_WEB_SPEED = 185;

    WebServer server(HTTP_PORT);
    String lastCommand = "none";

    String applyMotionCommand(const String &action, int speed)
    {
        int safeSpeed = constrain(speed, 0, 255);
        int arcSpeed = safeSpeed / 2;

        if (action == "forward")
        {
            setAutonomousDriveEnabled(false);
            driveTank(safeSpeed, safeSpeed);
            return "MOTION FORWARD";
        }
        if (action == "backward")
        {
            setAutonomousDriveEnabled(false);
            driveTank(-safeSpeed, -safeSpeed);
            return "MOTION BACKWARD";
        }
        if (action == "left")
        {
            setAutonomousDriveEnabled(false);
            driveTank(-safeSpeed, safeSpeed);
            return "MOTION LEFT";
        }
        if (action == "right")
        {
            setAutonomousDriveEnabled(false);
            driveTank(safeSpeed, -safeSpeed);
            return "MOTION RIGHT";
        }
        if (action == "forward_left")
        {
            setAutonomousDriveEnabled(false);
            driveTank(arcSpeed, safeSpeed);
            return "MOTION FORWARD_LEFT";
        }
        if (action == "forward_right")
        {
            setAutonomousDriveEnabled(false);
            driveTank(safeSpeed, arcSpeed);
            return "MOTION FORWARD_RIGHT";
        }
        if (action == "backward_left")
        {
            setAutonomousDriveEnabled(false);
            driveTank(-arcSpeed, -safeSpeed);
            return "MOTION BACKWARD_LEFT";
        }
        if (action == "backward_right")
        {
            setAutonomousDriveEnabled(false);
            driveTank(-safeSpeed, -arcSpeed);
            return "MOTION BACKWARD_RIGHT";
        }
        if (action == "stop")
        {
            setAutonomousDriveEnabled(false);
            stopMotors();
            return "MOTION STOP";
        }

        return "UNKNOWN";
    }

    String applyServoCommand(const String &target, const String &action)
    {
        if (target == "head")
        {
            if (action == "left")
            {
                setServoAutoPoseEnabled(false);
                setHeadServoAngle(0);
                return "SERVO HEAD LEFT";
            }
            if (action == "center")
            {
                setServoAutoPoseEnabled(false);
                setHeadServoAngle(90);
                return "SERVO HEAD CENTER";
            }
            if (action == "right")
            {
                setServoAutoPoseEnabled(false);
                setHeadServoAngle(180);
                return "SERVO HEAD RIGHT";
            }
        }

        if (target == "left_arm")
        {
            if (action == "up")
            {
                setServoAutoPoseEnabled(false);
                setLeftArmServoAngle(120);
                return "SERVO LEFT_ARM UP";
            }
            if (action == "center")
            {
                setServoAutoPoseEnabled(false);
                setLeftArmServoAngle(60);
                return "SERVO LEFT_ARM CENTER";
            }
            if (action == "down")
            {
                setServoAutoPoseEnabled(false);
                setLeftArmServoAngle(0);
                return "SERVO LEFT_ARM DOWN";
            }
        }

        if (target == "right_arm")
        {
            if (action == "up")
            {
                setServoAutoPoseEnabled(false);
                setRightArmServoAngle(120);
                return "SERVO RIGHT_ARM UP";
            }
            if (action == "center")
            {
                setServoAutoPoseEnabled(false);
                setRightArmServoAngle(60);
                return "SERVO RIGHT_ARM CENTER";
            }
            if (action == "down")
            {
                setServoAutoPoseEnabled(false);
                setRightArmServoAngle(0);
                return "SERVO RIGHT_ARM DOWN";
            }
        }

        return "UNKNOWN";
    }

    String applySystemCommand(const String &action)
    {
        if (action == "autonomous_on")
        {
            setAutonomousDriveEnabled(true);
            return "SYSTEM AUTONOMOUS ON";
        }

        if (action == "autonomous_off")
        {
            setAutonomousDriveEnabled(false);
            stopMotors();
            return "SYSTEM AUTONOMOUS OFF";
        }

        if (action == "pose_on")
        {
            setServoAutoPoseEnabled(true);
            return "SYSTEM POSE ON";
        }

        if (action == "pose_off")
        {
            setServoAutoPoseEnabled(false);
            return "SYSTEM POSE OFF";
        }

        return "UNKNOWN";
    }

    String mapAndApplyCommand(const String &target, const String &action, int speed)
    {
        if (target == "motion")
            return applyMotionCommand(action, speed);

        if (target == "head" || target == "left_arm" || target == "right_arm")
            return applyServoCommand(target, action);

        if (target == "system")
            return applySystemCommand(action);

        return "UNKNOWN";
    }

    void handleRoot()
    {
        server.send(200, "text/html", getPageHtml());
    }

    void handleCommand()
    {
        String target = server.arg("target");
        String action = server.arg("action");
        int speed = server.hasArg("speed") ? server.arg("speed").toInt() : DEFAULT_WEB_SPEED;

        String command = mapAndApplyCommand(target, action, speed);

        if (command == "UNKNOWN")
        {
            server.send(400, "text/plain", command);
            return;
        }

        lastCommand = command;
        Serial.print("[WEB CMD] ");
        Serial.println(command);
        server.send(200, "text/plain", command);
    }

    void handleStatus()
    {
        String status = "last=" + lastCommand;
        status += " | auto_drive=";
        status += isAutonomousDriveEnabled() ? "on" : "off";
        status += " | auto_pose=";
        status += isServoAutoPoseEnabled() ? "on" : "off";

        server.send(200, "text/plain", status);
    }
}

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

    if (!startRobotAccessPoint())
    {
        Serial.println("[ERROR] AP start failed. Rebooting in 5s.");
        delay(5000);
        ESP.restart();
    }

    Serial.print("[INFO] AP started. SSID: ");
    Serial.println(getRobotAccessPointSsid());
    Serial.print("[INFO] AP IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/cmd", HTTP_GET, handleCommand);
    server.on("/status", HTTP_GET, handleStatus);
    server.begin();

    Serial.println("[INFO] HTTP server started");

    Serial.println("Ready.");
}

// ═══════════════════════════════════════════════════════════════
//  Main loop
// ═══════════════════════════════════════════════════════════════

void loop()
{
    server.handleClient();
    updateAutonomousDrive();
    updateCharge();
    updateServoIOC();
    delay(10);
}
