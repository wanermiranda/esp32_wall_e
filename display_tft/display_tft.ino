// Wall-E Solar Charge Level Gauge
// Based on the chest display from Disney/Pixar's Wall-E
// For ESP32 + ST7735 TFT display

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// Pin definitions for ESP32
#define TFT_CS 5    // Chip select
#define TFT_RST 15  // Reset
#define TFT_DC 32   // Data/Command
#define TFT_MOSI 23 // SDA / Data
#define TFT_SCLK 18 // SCK / Clock

// ── Colour palette (Wall-E yellowish-green on black) ──────────────
// The Wall-E display uses a warm yellow-green, like old phosphor screens
#define BLACK 0x0000
#define WALLE_GREEN 0x9E66  // Main yellowish-green  (R≈158 G≈204 B≈50)
#define WALLE_DIM 0x5B40    // Dimmer shade for decorative elements
#define WALLE_DARK 0x3240   // Very dim for empty bar slots / outlines
#define WALLE_BRIGHT 0xCF28 // Brighter highlight for the sun glow

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ── Gauge layout constants ────────────────────────────────────────
// Screen is 160 x 128 in landscape (rotation 1)

// Total number of charge bars
#define NUM_BARS 10

// Bar area geometry
#define BAR_AREA_X 44 // Left edge of bar area (right of sun)
#define BAR_AREA_Y 52 // Top of bar area
#define BAR_W 104     // Width of each bar  (fills to right edge - margin)
#define BAR_H 6       // Height of each bar
#define BAR_GAP 2     // Gap between bars
#define BAR_AREA_BOTTOM (BAR_AREA_Y + NUM_BARS * (BAR_H + BAR_GAP) - BAR_GAP)

// Sun icon centre
#define SUN_CX 20
#define SUN_CY 78

// ── State ─────────────────────────────────────────────────────────
int chargeLevel = 0;      // Current charge (0 .. NUM_BARS)
int prevChargeLevel = -1; // Previous to allow partial redraws
bool charging = true;     // Direction of animation
unsigned long lastStep = 0;
#define STEP_INTERVAL 600 // ms between charge level changes

// ══════════════════════════════════════════════════════════════════
//  Drawing helpers
// ══════════════════════════════════════════════════════════════════

// Draw a small sun icon (circle + rays)
void drawSun(int cx, int cy, uint16_t color)
{
  int r = 7;
  // Filled circle (sun body)
  tft.fillCircle(cx, cy, r, color);

  // 8 rays around the sun
  int rayLen = 5;
  int rayGap = 2; // gap between circle edge and ray start
  for (int i = 0; i < 8; i++)
  {
    float angle = i * PI / 4.0;
    int x0 = cx + (int)((r + rayGap) * cos(angle));
    int y0 = cy + (int)((r + rayGap) * sin(angle));
    int x1 = cx + (int)((r + rayGap + rayLen) * cos(angle));
    int y1 = cy + (int)((r + rayGap + rayLen) * sin(angle));
    tft.drawLine(x0, y0, x1, y1, color);
  }
}

// Draw a single charge bar at slot index (0 = top bar, NUM_BARS-1 = bottom)
void drawBar(int index, bool filled)
{
  int y = BAR_AREA_Y + index * (BAR_H + BAR_GAP);
  if (filled)
  {
    tft.fillRect(BAR_AREA_X, y, BAR_W, BAR_H, WALLE_GREEN);
  }
  else
  {
    // Empty slot: dark outline only, like the movie
    tft.fillRect(BAR_AREA_X, y, BAR_W, BAR_H, BLACK);
    tft.drawRect(BAR_AREA_X, y, BAR_W, BAR_H, WALLE_DARK);
  }
}

// Draw the static parts of the gauge (call once)
void drawGaugeFrame()
{
  tft.fillScreen(BLACK);

  // ── Title text ──────────────────────────────────────────────────
  // "SOLAR CHARGE" on first line, "LEVEL" on second, centred-ish
  tft.setTextColor(WALLE_GREEN);
  tft.setTextSize(1);

  // First line
  const char *line1 = "SOLAR CHARGE";
  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(line1, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor((160 - tw) / 2, 6);
  tft.print(line1);

  // Second line
  const char *line2 = "LEVEL";
  tft.getTextBounds(line2, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor((160 - tw) / 2, 18);
  tft.print(line2);

  // Thin decorative line under the title
  tft.drawFastHLine(10, 32, 140, WALLE_DIM);
  tft.drawFastHLine(10, 33, 140, WALLE_DARK);

  // ── Decorative border (like a CRT bezel) ────────────────────────
  tft.drawRect(2, 2, 156, 124, WALLE_DIM);
  tft.drawRect(3, 3, 154, 122, WALLE_DARK);

  // ── Sun icon on the left side, vertically centred in bar area ───
  drawSun(SUN_CX, SUN_CY, WALLE_GREEN);

  // Small label under the sun
  tft.setTextSize(1);
  tft.setTextColor(WALLE_DIM);
  const char *sunLabel = "SOL";
  tft.getTextBounds(sunLabel, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor(SUN_CX - tw / 2, SUN_CY + 16);
  tft.print(sunLabel);

  // ── Thin vertical separator between sun and bars ────────────────
  tft.drawFastVLine(BAR_AREA_X - 5, BAR_AREA_Y - 4, BAR_AREA_BOTTOM - BAR_AREA_Y + 8, WALLE_DARK);

  // ── Draw label above bars ───────────────────────────────────────
  tft.setTextColor(WALLE_DIM);
  tft.setTextSize(1);
  tft.setCursor(BAR_AREA_X, BAR_AREA_Y - 12);
  tft.print(F("CHG"));

  // Right-aligned percentage placeholder
  tft.setCursor(BAR_AREA_X + BAR_W - 24, BAR_AREA_Y - 12);
  tft.print(F("  %"));
}

// Update the percentage text (right side above bars)
void drawPercentage(int pct)
{
  // Clear area
  tft.fillRect(BAR_AREA_X + BAR_W - 30, BAR_AREA_Y - 13, 30, 10, BLACK);
  tft.setTextColor(WALLE_GREEN);
  tft.setTextSize(1);

  char buf[5];
  sprintf(buf, "%3d%%", pct);
  tft.setCursor(BAR_AREA_X + BAR_W - 24, BAR_AREA_Y - 12);
  tft.print(buf);
}

// ══════════════════════════════════════════════════════════════════
//  Setup
// ══════════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  Serial.println(F("Wall-E Solar Charge Gauge starting..."));

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // Landscape 160x128

  drawGaugeFrame();

  // Draw all bars as empty initially
  for (int i = 0; i < NUM_BARS; i++)
  {
    drawBar(i, false);
  }
  drawPercentage(0);

  delay(500);
  Serial.println(F("Ready."));
}

// ══════════════════════════════════════════════════════════════════
//  Main loop – animate charge level up and down
// ══════════════════════════════════════════════════════════════════
void loop()
{
  unsigned long now = millis();
  if (now - lastStep < STEP_INTERVAL)
    return;
  lastStep = now;

  // Step the charge level
  if (charging)
  {
    chargeLevel++;
    if (chargeLevel >= NUM_BARS)
    {
      chargeLevel = NUM_BARS;
      charging = false; // start draining after a pause
      delay(2000);      // hold at full for 2 seconds
    }
  }
  else
  {
    chargeLevel--;
    if (chargeLevel <= 0)
    {
      chargeLevel = 0;
      charging = true; // start charging again after a pause
      delay(1500);     // hold at empty for 1.5 seconds
    }
  }

  // Only redraw the bars that changed
  if (chargeLevel != prevChargeLevel)
  {
    // Bars are drawn top-to-bottom (index 0 = top).
    // "Filled" bars go from the BOTTOM up, so bar index (NUM_BARS-1)
    // fills first, then (NUM_BARS-2), etc.
    for (int i = 0; i < NUM_BARS; i++)
    {
      bool shouldFill = (i >= (NUM_BARS - chargeLevel));
      bool wasFilled = (prevChargeLevel >= 0) && (i >= (NUM_BARS - prevChargeLevel));
      if (shouldFill != wasFilled)
      {
        drawBar(i, shouldFill);
      }
    }

    // Update percentage readout
    int pct = (chargeLevel * 100) / NUM_BARS;
    drawPercentage(pct);

    prevChargeLevel = chargeLevel;

    Serial.print(F("Charge: "));
    Serial.print(pct);
    Serial.println(F("%"));
  }
}