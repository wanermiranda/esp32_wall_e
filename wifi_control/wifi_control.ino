#include <WiFi.h>
#include <WebServer.h>

#include "wifi_ap.h"
#include "web_ui.h"

namespace
{
    constexpr uint16_t HTTP_PORT = 80;

    WebServer server(HTTP_PORT);

    String lastCommand = "none";

    String mapCommand(const String &target, const String &action, int speed)
    {
        if (target == "motion")
        {
            int safeSpeed = constrain(speed, 0, 255);

            if (action == "forward")
                return "MOTION FORWARD -> driveTank(" + String(safeSpeed) + "," + String(safeSpeed) + ")";
            if (action == "backward")
                return "MOTION BACKWARD -> driveTank(" + String(-safeSpeed) + "," + String(-safeSpeed) + ")";
            if (action == "left")
                return "MOTION LEFT -> driveTank(" + String(-safeSpeed) + "," + String(safeSpeed) + ")";
            if (action == "right")
                return "MOTION RIGHT -> driveTank(" + String(safeSpeed) + "," + String(-safeSpeed) + ")";
            if (action == "stop")
                return "MOTION STOP -> stopMotors()";
        }

        if (target == "head")
        {
            if (action == "left")
                return "SERVO HEAD -> 0";
            if (action == "center")
                return "SERVO HEAD -> 90";
            if (action == "right")
                return "SERVO HEAD -> 180";
        }

        if (target == "left_arm")
        {
            if (action == "up")
                return "SERVO LEFT_ARM -> 120";
            if (action == "center")
                return "SERVO LEFT_ARM -> 60";
            if (action == "down")
                return "SERVO LEFT_ARM -> 0";
        }

        if (target == "right_arm")
        {
            if (action == "up")
                return "SERVO RIGHT_ARM -> 120";
            if (action == "center")
                return "SERVO RIGHT_ARM -> 60";
            if (action == "down")
                return "SERVO RIGHT_ARM -> 0";
        }

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
        int speed = server.hasArg("speed") ? server.arg("speed").toInt() : 185;
        String command = mapCommand(target, action, speed);

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
        server.send(200, "text/plain", lastCommand);
    }

}

void setup()
{
    Serial.begin(115200);
    Serial.println("== WiFi Robot Control (Serial Logging) ==");

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
}

void loop()
{
    server.handleClient();
}
