#include "pico/stdlib.h"
#include "led.h"


#define TIMER_PHASES 4

static repeating_timer_t mLedTimer;

static int mLedDutyCycle = LED_PATTERN_OFF;
static int mTimerPhaseCounter = 0;

// switch on or off onboard LED
void setLed(bool _state) { gpio_put(PICO_DEFAULT_LED_PIN, _state); }

// set blink pattern off onboard LED
void setLedPattern(int _pattern) {
  mLedDutyCycle = _pattern;
  if (LED_PATTERN_OFF == _pattern) setLed(false);
}

// operate LED-blinking via timer
static bool repeating_timer_callback(__unused repeating_timer_t *_t) {
  if (LED_PATTERN_OFF != mLedDutyCycle) {
    if (mTimerPhaseCounter == mLedDutyCycle) setLed(true);
    else if (mTimerPhaseCounter == 0)       setLed(false);

    mTimerPhaseCounter = --mTimerPhaseCounter & (TIMER_PHASES-1);
  }

  return true;
}

void ledInit() {
  // initialize LED-pins
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  // initialize blink-timer
  add_repeating_timer_ms(250, repeating_timer_callback, NULL, &mLedTimer);
}
