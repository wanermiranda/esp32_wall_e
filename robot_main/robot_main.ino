/**
 * Robot Main — ESP32 WROOM
 *
 * Combines:
 *   • DC motor control via L298N (joystick-driven, differential drive)
 *   • Wall-E Solar Charge Level gauge on ST7735 TFT
 *
 * Pin conflict resolution (vs the standalone sketches):
 *   TFT_DC   32 → 2   (GPIO 32 needed for Motor B ENB)
 *   TFT_MOSI 23 → 13  (GPIO 23 needed for joystick SW)
 *   SW       23 → 4   (GPIO 23 freed for TFT, SW moved)
 */

#include <ezButton.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// ═══════════════════════════════════════════════════════════════
//  Pin definitions
// ═══════════════════════════════════════════════════════════════

// ─── Motor A (left) ──────────────────────────────────────────
#define ENA_PIN 14
#define IN1_PIN 27
#define IN2_PIN 26

// ─── Motor B (right) ─────────────────────────────────────────
#define ENB_PIN 32
#define IN3_PIN 25
#define IN4_PIN 33

// ─── Joystick (ADC1 pins for reliability) ────────────────────
#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 4

// ─── TFT ST7735 (software SPI) ──────────────────────────────
#define TFT_CS 5
#define TFT_RST 15
#define TFT_DC 2
#define TFT_MOSI 13
#define TFT_SCLK 18

// ═══════════════════════════════════════════════════════════════
//  Objects
// ═══════════════════════════════════════════════════════════════

ezButton button(SW_PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ═══════════════════════════════════════════════════════════════
//  Motor config
// ═══════════════════════════════════════════════════════════════

const int PWM_FREQ = 1000;
const int PWM_CHANNEL_A = 0;
const int PWM_CHANNEL_B = 1;
const int PWM_RESOLUTION = 8; // 0-255
const int DEADZONE = 15;

int centerX = 2048;
int centerY = 2048;
bool motorsEnabled = true;

// ═══════════════════════════════════════════════════════════════
//  TFT colour palette (Wall-E phosphor green)
// ═══════════════════════════════════════════════════════════════

#define BLACK 0x0000
#define WALLE_GREEN 0x9E66
#define WALLE_DIM 0x5B40
#define WALLE_DARK 0x3240
#define WALLE_BRIGHT 0xCF28

// ─── Gauge layout ────────────────────────────────────────────
#define NUM_BARS 12
#define SUN_CX 22
#define SUN_CY 55
#define BAR_AREA_X 44
#define BAR_AREA_Y 32
#define BAR_W 76
#define BAR_H 8
#define BAR_GAP 2
#define COL_SEP_X 40

int chargeLevel = 0;
int prevChargeLevel = -1;
bool charging = true;
unsigned long lastChargeStep = 0;
#define CHARGE_INTERVAL 600

// ═══════════════════════════════════════════════════════════════
//  Motor helpers
// ═══════════════════════════════════════════════════════════════

void setMotorA(bool fwd, uint8_t spd)
{
    digitalWrite(IN1_PIN, fwd ? HIGH : LOW);
    digitalWrite(IN2_PIN, fwd ? LOW : HIGH);
    ledcWrite(ENA_PIN, spd);
}

void setMotorB(bool fwd, uint8_t spd)
{
    digitalWrite(IN3_PIN, fwd ? HIGH : LOW);
    digitalWrite(IN4_PIN, fwd ? LOW : HIGH);
    ledcWrite(ENB_PIN, spd);
}

void stopMotors()
{
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, LOW);
    ledcWrite(ENA_PIN, 0);
    ledcWrite(ENB_PIN, 0);
}

// ═══════════════════════════════════════════════════════════════
//  Joystick
// ═══════════════════════════════════════════════════════════════

void readJoystick(int &vrx, int &vry)
{
    int rawX = analogRead(VRX_PIN);
    int rawY = analogRead(VRY_PIN);

    vrx = map(rawX, 0, 4095, -255, 255) - map(centerX, 0, 4095, -255, 255);
    vry = map(rawY, 0, 4095, -255, 255) - map(centerY, 0, 4095, -255, 255);
    vry = -vry; // forward = positive

    vrx = constrain(vrx, -255, 255);
    vry = constrain(vry, -255, 255);
}

void joystickControl(int vrx, int vry)
{
    if (abs(vrx) < DEADZONE)
        vrx = 0;
    if (abs(vry) < DEADZONE)
        vry = 0;

    if (vrx == 0 && vry == 0)
    {
        stopMotors();
        return;
    }

    int leftRaw = constrain(vry + vrx, -255, 255);
    int rightRaw = constrain(vry - vrx, -255, 255);

    setMotorA(leftRaw >= 0, (uint8_t)abs(leftRaw));
    setMotorB(rightRaw >= 0, (uint8_t)abs(rightRaw));
}

// ═══════════════════════════════════════════════════════════════
//  TFT drawing helpers
// ═══════════════════════════════════════════════════════════════

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
    int y = BAR_AREA_Y + index * (BAR_H + BAR_GAP);
    if (filled)
        tft.fillRect(BAR_AREA_X, y, BAR_W, BAR_H, WALLE_GREEN);
    else
    {
        tft.fillRect(BAR_AREA_X, y, BAR_W, BAR_H, BLACK);
        tft.drawRect(BAR_AREA_X, y, BAR_W, BAR_H, WALLE_DARK);
    }
}

void drawGaugeFrame()
{
    tft.fillScreen(BLACK);

    // Title
    tft.setTextColor(WALLE_GREEN);
    tft.setTextSize(1);
    const char *title = "SOLAR CHARGE LEVEL";
    int16_t x1, y1;
    uint16_t tw, th;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &tw, &th);
    tft.setCursor((128 - tw) / 2, 8);
    tft.print(title);

    // Decorations
    tft.drawFastHLine(8, 22, 112, WALLE_DIM);
    tft.drawFastHLine(8, 23, 112, WALLE_DARK);
    tft.drawRect(2, 2, 124, 156, WALLE_DIM);
    tft.drawRect(3, 3, 122, 154, WALLE_DARK);
    tft.drawFastVLine(COL_SEP_X, 26, 128, WALLE_DARK);

    drawSun(SUN_CX, SUN_CY, WALLE_GREEN);

    for (int i = 0; i < NUM_BARS; i++)
        drawBar(i, false);
}

void updateCharge()
{
    unsigned long now = millis();
    if (now - lastChargeStep < CHARGE_INTERVAL)
        return;
    lastChargeStep = now;

    if (charging)
    {
        chargeLevel++;
        if (chargeLevel >= NUM_BARS)
        {
            chargeLevel = NUM_BARS;
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
        for (int i = 0; i < NUM_BARS; i++)
        {
            bool shouldFill = (i >= (NUM_BARS - chargeLevel));
            bool wasFilled = (prevChargeLevel >= 0) && (i >= (NUM_BARS - prevChargeLevel));
            if (shouldFill != wasFilled)
                drawBar(i, shouldFill);
        }
        prevChargeLevel = chargeLevel;
    }
}

// ═══════════════════════════════════════════════════════════════
//  Setup
// ═══════════════════════════════════════════════════════════════

void setup()
{
    Serial.begin(115200);
    Serial.println("== Robot Main ==");

    // ── ADC ──────────────────────────────────────────────────
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    // ── Joystick button ──────────────────────────────────────
    button.setDebounceTime(50);

    // ── Calibrate joystick centre ────────────────────────────
    long sumX = 0, sumY = 0;
    for (int i = 0; i < 32; i++)
    {
        sumX += analogRead(VRX_PIN);
        sumY += analogRead(VRY_PIN);
        delay(5);
    }
    centerX = sumX / 32;
    centerY = sumY / 32;
    Serial.print("Joystick centre → X: ");
    Serial.print(centerX);
    Serial.print("  Y: ");
    Serial.println(centerY);

    // ── Motor pins ───────────────────────────────────────────
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(IN3_PIN, OUTPUT);
    pinMode(IN4_PIN, OUTPUT);
    ledcAttachChannel(ENA_PIN, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL_A);
    ledcAttachChannel(ENB_PIN, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL_B);
    stopMotors();

    // ── TFT ──────────────────────────────────────────────────
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(2);
    drawGaugeFrame();

    Serial.println("Ready.");
}

// ═══════════════════════════════════════════════════════════════
//  Main loop
// ═══════════════════════════════════════════════════════════════

void loop()
{
    button.loop();

    // ── SW toggle ────────────────────────────────────────────
    if (button.isPressed())
    {
        motorsEnabled = !motorsEnabled;
        if (!motorsEnabled)
        {
            stopMotors();
            Serial.println("MOTORS OFF");
        }
        else
        {
            Serial.println("MOTORS ON");
        }
    }

    // ── Motors ───────────────────────────────────────────────
    if (motorsEnabled)
    {
        int vrx, vry;
        readJoystick(vrx, vry);
        joystickControl(vrx, vry);
    }

    // ── TFT gauge animation ──────────────────────────────────
    updateCharge();

    delay(50);
}
