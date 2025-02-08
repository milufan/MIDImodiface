#include "bsp/board_api.h"

#include "fifo.h"
#include "midimodiface.h"
#include "led.h"
#include "uartmidi.h"
#include "usbmidi.h"
#include "midiprocessor.h"


semaphore_t mSemTasks2Exec;

static bool mLoopbackOperation = true;

// swich operation mode to loopback (incl. velocity processing)
void loopbackOn() {
  mLoopbackOperation = true;
  txLocalOff();
  setLedPattern(LED_PATTERN_OFF); 
}

// swich operation mode to MIDI-interface (forwarding in both directions)
void loopbackOff() {
  mLoopbackOperation = false;
  txLocalOn();
  setLedPattern(LED_PATTERN_75); 
}

int main() {
  // stdio_init_all();
  // printf("Hello, world!\n");

  // initialize board
  board_init();

  // initialize Semaphores
  sem_init(&mSemTasks2Exec, 0, 2*FIFO16_SIZE+1);

  // initialize UART
  uartMidiInit();

  // initialize LED
  ledInit();

  // initialize tinyUSB
  usbMidiInit();

  // processing-loop
  while (true) {
    if (mLoopbackOperation) filterMidi();
    else                    forwardMidi();
  }
}