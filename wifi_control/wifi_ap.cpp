#include <WiFi.h>

#include "wifi_ap.h"

namespace
{
    constexpr char AP_SSID[] = "ESP32-Robot-Control";
    constexpr char AP_PASSWORD[] = "robot1234";
}

bool startRobotAccessPoint()
{
    WiFi.mode(WIFI_AP);
    return WiFi.softAP(AP_SSID, AP_PASSWORD);
}

const char *getRobotAccessPointSsid()
{
    return AP_SSID;
}