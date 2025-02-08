#ifndef LED_H_
#define LED_H_

#define LED_PATTERN_OFF 4
#define LED_PATTERN_25  1
#define LED_PATTERN_50  2
#define LED_PATTERN_75  3
#define LED_PATTERN_ON  0

extern void setLed(bool);
extern void setLedPattern(int);
extern void ledInit();

#endif /* LED_H_*/
