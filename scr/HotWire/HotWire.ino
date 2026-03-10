#include <Adafruit_NeoPixel.h>
#include <MD_MAX72xx.h >
#include "pitches.h"
#include "definitions.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2
#define CLK_PIN 13     // or SCK
#define DATA_PIN 11    // or MOSI
#define CS_PIN 10      // or SS
#define DELAYTIME 100  // in milliseconds


#define LED_PIN 2
#define LED_COUNT 6
#define BUZZER_PIN 3
#define START_BUTTON_PIN 4
#define CONTACT_BUTTON_PIN 5

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

enum { STANDBY,
       PRE_RUNNING,
       RUNNING,
       GAME_OVER,
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting System");

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CONTACT_BUTTON_PIN, INPUT_PULLUP);

  strip.begin();               // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                // Turn OFF all pixels ASAP
  strip.setBrightness(255);    // Set BRIGHTNESS to about 1/5 (max = 255)
  randomSeed(analogRead(A0));  // bessere Zufallswerte

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 5);  // Helligkeit 0–15 mx.clear();
  showBigNumber(0);
}

uint8_t state = STANDBY;
bool contactPressed = false;
bool startPressed = false;
int lifeLights = 0;
int timerCounter = 0;
bool normalTime = true;
int delayValue = 0;

void loop() {
  if (!digitalRead(START_BUTTON_PIN)) {
    Serial.println("Start pressed 1!");
    delay(1000);
    if (!digitalRead(START_BUTTON_PIN)) {
      startPressed = true;
      normalTime = false;
      Serial.println("Start pressed 2!");
    }
    else
    {
      startPressed = true;
      normalTime = true;
    }
  }

  if (!digitalRead(CONTACT_BUTTON_PIN)) {
    contactPressed = true;
    Serial.println("Contact pressed!");
  }

  switch (state) {
    case STANDBY:
      if (startPressed) {
        startPressed = false;
        state = PRE_RUNNING;
        Serial.println("Go to PRE_RUNNING");
      } else {
        if (delayValue++ > 100) {
          delayValue = 0;
          randomPattern();
        }
        delay(10);
      }
      break;

    case PRE_RUNNING:
      for (int i = 6; i >= 0; i--) {
        ledShowNumber(i);
        Serial.print("LedShowNumber: ");
        Serial.println(i);
        tone(BUZZER_PIN, 98, 250);
        delay(500);
        noTone(BUZZER_PIN);
        delay(500);
      }
      lifeLights = 6;
      ledShowGreen(lifeLights);
      state = RUNNING;
      contactPressed = false;
      if(normalTime) timerCounter = 30;
      else timerCounter = 60;
      delayValue = 0;
      showBigNumber(timerCounter);
      Serial.println("Going to RUNNING");
      break;

    case RUNNING:
      Serial.println("RUNNING");
      if (contactPressed) {
        contactPressed = false;
        Serial.println("Contact pressed detected!");
        if (lifeLights > 0) {
          lifeLights--;
          Serial.print("lifeLights: ");
          Serial.println(lifeLights);
        }

        tone(BUZZER_PIN, 220, 250);
        delay(500);
        noTone(BUZZER_PIN);
        ledBlinkRed();
        ledShowGreen(lifeLights);
      }

      if (lifeLights == 0) {
        state = GAME_OVER;
        timerCounter = 0;
        showBigNumber(timerCounter);
      }

      if (delayValue++ > 100) {
        delayValue = 0;
        if (timerCounter > 0) timerCounter--;
        else state = GAME_OVER;
        showBigNumber(timerCounter);
      }

      if (startPressed) {
        startPressed = false;
        state = GAME_OVER;
      }

      delay(10);
      break;

    case GAME_OVER:
      playMusic();
      colorWipe(strip.Color(255, 0, 255), 250);
      colorWipe(strip.Color(0, 255, 255), 250);
      colorWipe(strip.Color(0, 0, 255), 250);
      delay(2000);
      state = STANDBY;
      contactPressed = false;
      startPressed = false;
      timerCounter = 0;
      showBigNumber(timerCounter);
      break;
  }
}


void randomPattern() {
  strip.clear();
  uint32_t orange = strip.Color(255, 80, 0);  // Beispiel: Zufälliges Pattern auswählen
  int pattern = random(3);                    // 0,1,2
  switch (pattern) {
    case 0:
      patternSingle(orange);
      break;

    case 1:
      patternRandomPixels(orange);
      break;

    case 2:
      patternHalf(orange);
      break;
  }
  strip.show();
}

// --- Pattern 1: genau 1 zufälliger Pixel ---
void patternSingle(uint32_t color) {
  int p = random(LED_COUNT);
  strip.setPixelColor(p, color);
}

// --- Pattern 2: jedes Pixel hat 50% Chance ---
void patternRandomPixels(uint32_t color) {
  for (int i = 0; i < LED_COUNT; i++) {
    if (random(2) == 1) { strip.setPixelColor(i, color); }
  }
}

// --- Pattern 3: zufällige Hälfte der LEDs ---
void patternHalf(uint32_t color) {
  int count = LED_COUNT / 2;
  for (int i = 0; i < count; i++) {
    int p = random(LED_COUNT);
    strip.setPixelColor(p, color);
  }
}

void ledShowNumber(int number) {
  if (number > LED_COUNT) number = LED_COUNT;
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  for (int i = 0; i < number; i++) {
    strip.setPixelColor(i, strip.Color(255, 80, 0));
  }
  strip.show();
}

void ledShowGreen(int number) {
  if (number > LED_COUNT) number = LED_COUNT;
  if (number < 0) number = 0;
  int x = number--;

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  if (x >= 0) {
    for (int i = 0; i < x; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
  }
  strip.show();
}

void ledBlinkRed() {
  for (int x = 0; x < 2; x++) {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
    delay(100);
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
    strip.show();
    delay(100);
  }
}

void playMusic(void) {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_PIN);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
void drawBigDigit(uint8_t device, uint8_t digit) {
  for (uint8_t row = 0; row < 8; row++) { mx.setRow(device, row, bigDigits[digit][row]); }
}

void showBigNumber(int num) {
  drawBigDigit(0, num / 10);
  drawBigDigit(1, num % 10);
}

////////////////////////////////////////////////////////////////////////////////////////////

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                //  Update strip to match
    delay(wait);                                 //  Pause for a moment
  }
}
