/**
 * Control DC Motors — ESP32 WROOM + L298N
 *
 * Analog joystick input (VRX / VRY) with differential-drive
 * mixing and proportional speed control.
 *
 * Axis mapping (each axis range: -255 … 0 … +255):
 *   VRX  → negative = LEFT,  positive = RIGHT
 *   VRY  → negative = BACK,  positive = FRONT
 *
 * Combinations are supported:
 *   FRONT+LEFT, FRONT+RIGHT, BACK+LEFT, BACK+RIGHT
 *   or any single direction alone.
 *
 * Speed is derived from the magnitude of the joystick
 * displacement on each axis, so partial tilt = slower speed.
 */

#include <ezButton.h>

// ─── Motor A (left motor) ────────────────────────────────────
#define ENA_PIN 14 // ENA — PWM speed
#define IN1_PIN 27 // IN1 — direction
#define IN2_PIN 26 // IN2 — direction

// ─── Motor B (right motor) ───────────────────────────────────
#define ENB_PIN 32 // ENB — PWM speed
#define IN3_PIN 25 // IN3 — direction
#define IN4_PIN 33 // IN4 — direction

// ─── Joystick (VRX/VRY on ADC1 for reliability) ─────────────
#define VRX_PIN 34 // Analog — horizontal axis
#define VRY_PIN 35 // Analog — vertical axis
#define SW_PIN 23  // Digital — push-button (active LOW)

ezButton button(SW_PIN); // ezButton handles debounce internally

// ESP32 ADC: 12-bit (0-4095). Centre is calibrated at startup.
int centerX = 2048;
int centerY = 2048;

// ─── PWM config ──────────────────────────────────────────────
const int PWM_FREQ = 1000; // Hz
const int PWM_CHANNEL_A = 0;
const int PWM_CHANNEL_B = 1;
const int PWM_RESOLUTION = 8; // 8-bit → 0-255

// ─── Dead-zone threshold (ignore tiny joystick drift) ────────
const int DEADZONE = 15;

// ─────────────────────────────────────────────────────────────
//  Low-level helpers
// ─────────────────────────────────────────────────────────────

/** Set motor A direction and speed (speed 0-255). */
void setMotorA(bool forward, uint8_t speed)
{
    digitalWrite(IN1_PIN, forward ? HIGH : LOW);
    digitalWrite(IN2_PIN, forward ? LOW : HIGH);
    ledcWrite(ENA_PIN, speed);
}

/** Set motor B direction and speed (speed 0-255). */
void setMotorB(bool forward, uint8_t speed)
{
    digitalWrite(IN3_PIN, forward ? HIGH : LOW);
    digitalWrite(IN4_PIN, forward ? LOW : HIGH);
    ledcWrite(ENB_PIN, speed);
}

/** Stop both motors immediately. */
void stopMotors()
{
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, LOW);
    ledcWrite(ENA_PIN, 0);
    ledcWrite(ENB_PIN, 0);
}

// ─────────────────────────────────────────────────────────────
//  Joystick analog reading
// ─────────────────────────────────────────────────────────────

/**
 * Read the joystick and map ADC values (0-4095) to signed range
 * (–255 … +255) centred on the ADC midpoint.
 *
 * @param[out] vrx  Horizontal: –255 (left) … +255 (right)
 * @param[out] vry  Vertical:   –255 (back) … +255 (front)
 */
void readJoystick(int &vrx, int &vry)
{
    int rawX = analogRead(VRX_PIN);
    int rawY = analogRead(VRY_PIN);

    // Log raw ADC values for debugging
    Serial.print("[RAW] X: ");
    Serial.print(rawX);
    Serial.print("  Y: ");
    Serial.print(rawY);
    Serial.print("  (center X: ");
    Serial.print(centerX);
    Serial.print("  Y: ");
    Serial.print(centerY);
    Serial.println(")");

    // Simple signed offset from calibrated centre, scaled to -255..+255
    vrx = map(rawX, 0, 4095, -255, 255) - map(centerX, 0, 4095, -255, 255);
    vry = map(rawY, 0, 4095, -255, 255) - map(centerY, 0, 4095, -255, 255);

    // Invert VRY so that pushing the stick forward = positive
    vry = -vry;

    vrx = constrain(vrx, -255, 255);
    vry = constrain(vry, -255, 255);
}

// ─────────────────────────────────────────────────────────────
//  Joystick control (VRX / VRY)
// ─────────────────────────────────────────────────────────────

/**
 * Drive the robot with analog joystick axes.
 *
 * @param vrx  Horizontal axis  –255 (full left)  … 0 … +255 (full right)
 * @param vry  Vertical   axis  –255 (full back)  … 0 … +255 (full front)
 *
 * Mixing algorithm (differential-drive):
 *   leftSpeed  = vry + vrx   → left motor
 *   rightSpeed = vry – vrx   → right motor
 *
 * The sign of each result sets direction; the magnitude sets PWM duty.
 * Values are clamped to [–255, 255].
 *
 * Examples:
 *   joystickControl(   0,  255)  → full speed FRONT
 *   joystickControl(   0, -255)  → full speed BACK
 *   joystickControl(-255,    0)  → spin LEFT
 *   joystickControl( 255,    0)  → spin RIGHT
 *   joystickControl( 128,  200)  → curve FRONT-RIGHT
 *   joystickControl(-100, -180)  → curve BACK-LEFT
 */
void joystickControl(int vrx, int vry)
{
    // Apply dead-zone
    if (abs(vrx) < DEADZONE)
        vrx = 0;
    if (abs(vry) < DEADZONE)
        vry = 0;

    // Nothing to do
    if (vrx == 0 && vry == 0)
    {
        stopMotors();
        Serial.println("STOP");
        return;
    }

    // Differential-drive mixing
    int leftRaw = vry + vrx;
    int rightRaw = vry - vrx;

    // Clamp to [-255, 255]
    leftRaw = constrain(leftRaw, -255, 255);
    rightRaw = constrain(rightRaw, -255, 255);

    // Direction and speed from sign and magnitude
    bool leftFwd = (leftRaw >= 0);
    bool rightFwd = (rightRaw >= 0);
    uint8_t leftSpeed = (uint8_t)abs(leftRaw);
    uint8_t rightSpeed = (uint8_t)abs(rightRaw);

    setMotorA(leftFwd, leftSpeed);
    setMotorB(rightFwd, rightSpeed);

    // ── Debug output ───────────────────────────────────────────
    Serial.print("VRX: ");
    Serial.print(vrx);
    Serial.print("  VRY: ");
    Serial.print(vry);
    Serial.print(" | L: ");
    Serial.print(leftFwd ? "FWD " : "REV ");
    Serial.print(leftSpeed);
    Serial.print(" | R: ");
    Serial.print(rightFwd ? "FWD " : "REV ");
    Serial.println(rightSpeed);
}

// ─────────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────────

void setup()
{
    Serial.begin(115200);
    Serial.println("== Control DC Motors — Joystick ==");

    // ── ADC configuration ────────────────────────────────────────
    analogSetAttenuation(ADC_11db); // Full 0–3.3 V range
    analogReadResolution(12);       // 12-bit: 0-4095
    // GPIO 34/35 are input-only — no pinMode needed for analogRead

    // SW button via ezButton (handles pull-up & debounce)
    button.setDebounceTime(50);

    // ── Calibrate joystick centre (average multiple reads) ────
    // Leave the stick untouched during startup!
    long sumX = 0, sumY = 0;
    const int samples = 32;
    for (int i = 0; i < samples; i++)
    {
        sumX += analogRead(VRX_PIN);
        sumY += analogRead(VRY_PIN);
        delay(5);
    }
    centerX = sumX / samples;
    centerY = sumY / samples;
    Serial.print("Joystick centre calibrated → X: ");
    Serial.print(centerX);
    Serial.print("  Y: ");
    Serial.println(centerY);

    // Direction pins
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(IN3_PIN, OUTPUT);
    pinMode(IN4_PIN, OUTPUT);

    // PWM channels (ESP32 Arduino 3.x API)
    ledcAttachChannel(ENA_PIN, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL_A);
    ledcAttachChannel(ENB_PIN, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL_B);

    // Start stopped
    stopMotors();
}

// ─────────────────────────────────────────────────────────────
//  Main loop — read joystick and drive motors in real time
// ─────────────────────────────────────────────────────────────

bool motorsEnabled = true; // SW button toggles this

void loop()
{
    button.loop(); // must be called each iteration

    // ── SW button toggles motor enable ─────────────────────────
    if (button.isPressed())
    {
        motorsEnabled = !motorsEnabled;
        if (!motorsEnabled)
        {
            stopMotors();
            Serial.println(">> MOTORS DISABLED (press SW to re-enable)");
        }
        else
        {
            Serial.println(">> MOTORS ENABLED");
        }
    }

    if (!motorsEnabled)
    {
        delay(50);
        return;
    }

    // ── Read joystick axes ─────────────────────────────────────
    int vrx, vry;
    readJoystick(vrx, vry);

    // ── Drive motors ───────────────────────────────────────────
    joystickControl(vrx, vry);

    delay(50); // ~20 Hz update rate
}
