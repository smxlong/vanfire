#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    300
#define TIMESTEP    20
#define LED_TYPE    WS2813
#define COLOR_ORDER GRB

uint8_t GlobalBrightness = 255;

CRGB leds[NUM_LEDS];

namespace my {
  template <typename T>
  T Min(T a, T b) {
    return (a <= b) ? a : b;
  }
}

class Plane {
public:
  virtual void Render(CRGB* inout, uint16_t timestep) = 0;
};

inline CRGB FirePalette1(uint8_t x) {
  return CHSV(x / 5, 255, my::Min(255, x * 2));
}

class FireSparkPlane : public Plane {
public:
  const uint8_t chance = 6;
  const uint8_t magic = 2;

  virtual void Render(CRGB* inout, uint16_t timestep) {
    for (unsigned int i = 0; i < NUM_LEDS; ++i) {
      if (random8() < chance) {
        if (random8() < magic) {
          inout[i] = CRGB(0, 255, 80);
        } else {
          inout[i] = FirePalette1(255);
        }
      }
    }
  }
};

inline CRGB Blend3(const CRGB& a, int ac, const CRGB& b, int bc, const CRGB& c, int cc, int d) {
  int tr = a.red * ac / d + b.red * bc / d + c.red * cc / d;
  int tg = a.green * ac / d + b.green * bc / d + c.green * cc / d;
  int tb = a.blue * ac / d + b.blue * bc / d + c.blue * cc / d;
  return CRGB(tr, tg, tb);
}

class BlurPlane : public Plane {
public:
  virtual void Render(CRGB* inout, uint16_t timestep) {
    const int AC = 8;
    const int BC = 17;
    const int CC = 8;
    const int D = 32;
    CRGB a(0, 0, 0);
    CRGB b = inout[0];
    CRGB c = inout[1];
    for (unsigned int i = 0; i < NUM_LEDS - 2; ++i) {
      inout[i] = Blend3(a, AC, b, BC, c, CC, D);
      a = b;
      b = c;
      c = inout[i + 2];
    }
    inout[NUM_LEDS - 2] = Blend3(a, AC, b, BC, c, CC, D);
    inout[NUM_LEDS - 1] = Blend3(b, AC, c, BC, CRGB(), 0, D);
  }
};

class FireFadePlane : public Plane {
public:
  virtual void Render(CRGB* inout, uint16_t timestep) {
    for (unsigned int i = 0; i < NUM_LEDS; ++i) {
      inout[i].red = inout[i].red * 7 / 8;
      inout[i].green = inout[i].green * 5 / 8;
      inout[i].blue = inout[i].blue * 1 / 8;
    }
  }
};

class FlickerPlane : public Plane {
public:
  const uint8_t chance = 100;

  virtual void Render(CRGB* inout, uint16_t timestep) {
    if (random8() < chance) {
      if (random8() & 1) {
        // Flicker left
        for (unsigned int i = 0; i < NUM_LEDS - 1; ++i) {
          inout[i] = inout[i + 1];
        }
        inout[NUM_LEDS - 1] = CRGB(0, 0, 0);
      } else {
        // Flicker right
        for (unsigned int i = NUM_LEDS - 1; i > 0; --i) {
          inout[i] = inout[i - 1];
        }
        inout[0] = CRGB(0, 0, 0);
      }
    }
  }
};

FireSparkPlane plane_FireSpark;
FireFadePlane plane_FireFade;
BlurPlane plane_Blur;
FlickerPlane plane_Flicker;

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  delay(1000);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(GlobalBrightness);
}

void loop() {
  plane_Blur.Render(leds, TIMESTEP);
  plane_Flicker.Render(leds, TIMESTEP);
  plane_Blur.Render(leds, TIMESTEP);
  plane_FireFade.Render(leds, TIMESTEP);
  plane_FireSpark.Render(leds, TIMESTEP);
  FastLED.show();
  if (TIMESTEP) {
    delay(TIMESTEP);
  }
}

