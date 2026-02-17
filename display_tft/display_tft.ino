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
// Screen is 128 x 160 in portrait (rotation 2)
// Two-column layout below title:
//   Left column  (x 6..38)  : sun icon + percentage
//   Right column (x 44..122): charge bars

// Total number of charge bars
#define NUM_BARS 12

// Left column (sun + percentage)
#define LEFT_COL_CX 22     // Centre X of left column
#define SUN_CX LEFT_COL_CX // Sun icon centre X
#define SUN_CY 55          // Sun icon centre Y

// Right column (bars)
#define BAR_AREA_X 44 // Left edge of bar area
#define BAR_AREA_Y 32 // Top of first bar
#define BAR_W 76      // Width of each bar
#define BAR_H 8       // Height of each bar
#define BAR_GAP 2     // Gap between bars
#define BAR_AREA_BOTTOM (BAR_AREA_Y + NUM_BARS * (BAR_H + BAR_GAP) - BAR_GAP)

// Vertical separator X between columns
#define COL_SEP_X 40

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

  // ── Title text (single line, centred) ───────────────────────────
  tft.setTextColor(WALLE_GREEN);
  tft.setTextSize(1); // 6x8 font → "SOLAR CHARGE LEVEL" fits 128px

  const char *title = "SOLAR CHARGE LEVEL";
  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor((128 - tw) / 2, 8);
  tft.print(title);

  // Thin decorative line under the title
  tft.drawFastHLine(8, 22, 112, WALLE_DIM);
  tft.drawFastHLine(8, 23, 112, WALLE_DARK);

  // ── Decorative border (CRT bezel) ──────────────────────────────
  tft.drawRect(2, 2, 124, 156, WALLE_DIM);
  tft.drawRect(3, 3, 122, 154, WALLE_DARK);

  // ── Vertical separator between left and right columns ──────────
  tft.drawFastVLine(COL_SEP_X, 26, 128, WALLE_DARK);

  // ── Left column: Sun icon ──────────────────────────────────────
  drawSun(SUN_CX, SUN_CY, WALLE_GREEN);
}

// ══════════════════════════════════════════════════════════════════
//  Setup
// ══════════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  Serial.println(F("Wall-E Solar Charge Gauge starting..."));

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2); // Portrait 128x160

  drawGaugeFrame();

  // Draw all bars as empty initially
  for (int i = 0; i < NUM_BARS; i++)
  {
    drawBar(i, false);
  }

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

    prevChargeLevel = chargeLevel;

    Serial.print(F("Charge: "));
    Serial.print((chargeLevel * 100) / NUM_BARS);
    Serial.println(F("%"));
  }
}