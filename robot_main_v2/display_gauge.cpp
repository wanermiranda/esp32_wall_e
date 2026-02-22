#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "display_gauge.h"
#include "robot_constants.h"

namespace
{
    Adafruit_ST7735 tft = Adafruit_ST7735(
        RobotPins::TFT_CS,
        RobotPins::TFT_DC,
        RobotPins::TFT_MOSI,
        RobotPins::TFT_SCLK,
        RobotPins::TFT_RST);

    int chargeLevel = 0;
    int prevChargeLevel = -1;
    bool charging = true;
    unsigned long lastChargeStep = 0;

    void drawSun(int cx, int cy, uint16_t color)
    {
        tft.fillCircle(cx, cy, 7, color);
        for (int i = 0; i < 8; i++)
        {
            float a = i * PI / 4.0;
            int x0 = cx + (int)(9 * cos(a));
            int y0 = cy + (int)(9 * sin(a));
            int x1 = cx + (int)(14 * cos(a));
            int y1 = cy + (int)(14 * sin(a));
            tft.drawLine(x0, y0, x1, y1, color);
        }
    }

    void drawBar(int index, bool filled)
    {
        int y = RobotConst::BAR_AREA_Y + index * (RobotConst::BAR_H + RobotConst::BAR_GAP);
        if (filled)
            tft.fillRect(RobotConst::BAR_AREA_X, y, RobotConst::BAR_W, RobotConst::BAR_H, RobotConst::WALLE_GREEN);
        else
        {
            tft.fillRect(RobotConst::BAR_AREA_X, y, RobotConst::BAR_W, RobotConst::BAR_H, RobotConst::BLACK);
            tft.drawRect(RobotConst::BAR_AREA_X, y, RobotConst::BAR_W, RobotConst::BAR_H, RobotConst::WALLE_DARK);
        }
    }

    void drawGaugeFrame()
    {
        tft.fillScreen(RobotConst::BLACK);

        tft.setTextColor(RobotConst::WALLE_GREEN);
        tft.setTextSize(1);
        const char *title = "SOLAR CHARGE LEVEL";
        int16_t x1, y1;
        uint16_t tw, th;
        tft.getTextBounds(title, 0, 0, &x1, &y1, &tw, &th);
        tft.setCursor((128 - tw) / 2, 8);
        tft.print(title);

        tft.drawFastHLine(8, 22, 112, RobotConst::WALLE_DIM);
        tft.drawFastHLine(8, 23, 112, RobotConst::WALLE_DARK);
        tft.drawRect(2, 2, 124, 156, RobotConst::WALLE_DIM);
        tft.drawRect(3, 3, 122, 154, RobotConst::WALLE_DARK);
        tft.drawFastVLine(RobotConst::COL_SEP_X, 26, 128, RobotConst::WALLE_DARK);

        drawSun(RobotConst::SUN_CX, RobotConst::SUN_CY, RobotConst::WALLE_GREEN);

        for (int i = 0; i < RobotConst::NUM_BARS; i++)
            drawBar(i, false);
    }
}

void initGaugeDisplay()
{
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(2);
    drawGaugeFrame();
}

void updateCharge()
{
    unsigned long now = millis();
    if (now - lastChargeStep < RobotConst::CHARGE_INTERVAL)
        return;
    lastChargeStep = now;

    if (charging)
    {
        chargeLevel++;
        if (chargeLevel >= RobotConst::NUM_BARS)
        {
            chargeLevel = RobotConst::NUM_BARS;
            charging = false;
        }
    }
    else
    {
        chargeLevel--;
        if (chargeLevel <= 0)
        {
            chargeLevel = 0;
            charging = true;
        }
    }

    if (chargeLevel != prevChargeLevel)
    {
        for (int i = 0; i < RobotConst::NUM_BARS; i++)
        {
            bool shouldFill = (i >= (RobotConst::NUM_BARS - chargeLevel));
            bool wasFilled = (prevChargeLevel >= 0) && (i >= (RobotConst::NUM_BARS - prevChargeLevel));
            if (shouldFill != wasFilled)
                drawBar(i, shouldFill);
        }
        prevChargeLevel = chargeLevel;
    }
}
