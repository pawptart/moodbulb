#include <Adafruit_NeoPixel.h>
#include <Adafruit_DotStar.h>
#include <Adafruit_TCS34725.h>
#ifdef __AVR__
 #include <avr/power.h>     // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN       4
#define LED_COUNT     1
#define BRIGHTNESS  255     // NeoPixel brightness, 0 (min) to 255 (max)
#define T_DURATION 5000     // Color transition duration, milliseconds
#define T_RATE       50     // Color transition rate, milliseconds
#define R_DELAY   30000     // Delay between color reads, milliseconds

// Initialize Neopixel object:
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Initialize TCS sensor:
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

// Initialize Trinket onboard Dotstar LED to turn off later:
Adafruit_DotStar onboardLED = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

uint32_t ledColor;          // Global store for current LED color

void setup() {
  /* Ensure that the TCS sensor is connected and can be detected
   *  
   * Otherwise, halt execution here
   */
  if (tcs.begin()) {
    Serial.println("Found TCS34725 sensor");
    tcs.setInterrupt(1);    // Make sure the LED initializes to off state
  } else {
    Serial.println("No TCS34725 sensor found ... check connection");
    while (1);
  }

  onboardLED.show();        // Turn off the onboard LED

  pixels.begin();           // Initialize pixel(s)
  pixels.setBrightness(BRIGHTNESS);
  pixels.show();            // No pixel data yet, turn OFF all pixels
}

void loop() {
  uint32_t rgb = getTargetColor();

  transitionToColor(rgb);   // Transition to the target color 

  delay(R_DELAY);
}

void transitionToColor(uint32_t rgb) {
  /*  Find the number of "ticks" we need to transition (i.e. the duration / rate)
   * 
   *  Subtract 1 from this value -- the last tick will be the desired color
   *  Each tick, calculate the RGB value based on the number of elapsed ticks
   *  The last tick is outside the loop, no need to calculate RGB since we already have it as a param
   */
  int ticks = T_DURATION / T_RATE - 1;

  float t_r = (rgb      >> 16) & 0xFF;
  float t_g = (rgb      >>  8) & 0xFF;
  float t_b =  rgb             & 0xFF;

  float c_r = (ledColor >> 16) & 0xFF;
  float c_g = (ledColor >>  8) & 0xFF;
  float c_b =  ledColor        & 0xFF;

  float tick_r = (t_r - c_r) / ticks;
  float tick_g = (t_g - c_g) / ticks;
  float tick_b = (t_b - c_b) / ticks;

  for (int i = 0; i < ticks; i++) {
    pixels.setPixelColor(0, c_r + tick_r * i, c_g + tick_g * i, c_b + tick_b * i);
    pixels.show();
    delay(T_RATE);
  }

  pixels.setPixelColor(0, rgb);
  pixels.show();
  ledColor = rgb;           // Set new ledColor state
}

uint32_t getTargetColor() {
  tcs.setInterrupt(0);      // Turn on LED
  delay(50);                // Delay 50ms to ensure surface illuminated during read

  float r, g, b;            // Initialize the colors

  tcs.getRGB(&r, &g, &b);   // Get RGB values from sensor
  logColor(r, g, b);        // Log 'em!

  tcs.setInterrupt(1);      // Turn LED off

  return pixels.Color(r, g, b);
}

void logColor(float r, float g, float b) {
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.println(" ");
}
