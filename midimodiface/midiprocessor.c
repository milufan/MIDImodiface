#include "bsp/board_api.h"
#include "fifo.h"
#include "midiprocessor.h"
#include "led.h"
#include "midimodiface.h"
#include "uartmidi.h"
#include "usbmidi.h"


// MIDI-Defindes
#define MIDI_COMMAND      0x80
#define MIDI_COMMAND_MASK 0xF0
#define MIDI_CHANNEL_MASK 0x0F
#define MIDI_MAXDATA      127

#define MIDI_NOTE_ON          0x90
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_SYSTEM_EXCLUSIVE 0xF0
#define MIDI_SYS_EX_END       0xF7
#define MIDI_ACTIVE_SENSING   0xFE

static uint8_t mLocalOff[] = {MIDI_CONTROL_CHANGE, 122, 0};
static uint8_t mlocalOn[] = {MIDI_CONTROL_CHANGE, 122, 127};

static bool mInitialized = false;
static bool mMaxVelocity = false;
static bool mSysExActive = false;
static bool mLedState = false;

// is velocity value valid?
static bool isValidVel(uint8_t _midibyte) {
  return (0 < _midibyte) && (_midibyte <= MIDI_MAXDATA);
}

// is input valid command?
static bool isCommand(uint8_t _midibyte) {
  return (_midibyte & MIDI_COMMAND) > 0;
}

// denotes program number a dynamic sound?
static bool isDynamicProg(uint8_t _midibyte) {
  return (12 != _midibyte) && (21 != _midibyte) && (22 != _midibyte) && (23 != _midibyte);
}

// is input part of a sysex-string?
static bool isSysExString(uint8_t _midibyte) {
  return mSysExActive || (MIDI_SYS_EX_END == _midibyte);
}

// transmit MIDI-bytes without sysex-strings via UART-interface
static void midi_tx_filtered(uint8_t _midibyte) {
  if (!isSysExString(_midibyte)) midi_tx(_midibyte);
}

// transmit local-off command via UART-interface
void txLocalOff() { txMidiString(mLocalOff, sizeof(mLocalOff)); }

// transmit local-on command
void txLocalOn() { txMidiString(mlocalOn, sizeof(mlocalOn)); }

// process note-on commands in loopback mode
static uint8_t handleNoteOn(uint8_t _midibyte) {
  // in through-mode switch off LED
  if (!mMaxVelocity && mLedState)
    setLed(mLedState = false);

  // tx NoteOn
  midi_tx_filtered(_midibyte);

  // get midi-note-on-key data
  _midibyte = midi_rx();

  // check it is not a new midi command (highly unlikely)
  if (!isCommand(_midibyte)) {

    // tx midi-note-on-key data
    midi_tx_filtered(_midibyte);

    // get midi-note-on-velocity data
    _midibyte = midi_rx();

    // check it is not a new midi command (highly unlikely)
    // nor midi-note-on-velocity-0 which is same as note-off
    if (isValidVel(_midibyte) && mMaxVelocity) {
      // alter midi-velocity
      _midibyte = MIDI_MAXDATA;
    }
  }

  return _midibyte;
}

// process program-change commands in loopback mode
static uint8_t handleProgramChange(uint8_t _midibyte) {
  txLocalOff();

  // tx Program change
  midi_tx_filtered(_midibyte);

  // get midi-program data
  _midibyte = midi_rx();

  if (isDynamicProg(_midibyte)) {
    if (mMaxVelocity)  setLed(mLedState = mMaxVelocity = false);
  } else {
    if (!mMaxVelocity) setLed(mLedState = mMaxVelocity = true);
  }

  return _midibyte;
}

// process active-sensing commands in loopback mode
static uint8_t handleActiveSensing(uint8_t _midibyte) {
  if (!mInitialized) {
    txLocalOff();
    mInitialized = true;
  }

  if (mMaxVelocity) mLedState = true;
  else              mLedState = !mLedState;
  setLed(mLedState);

  return _midibyte;
}

// filter MIDI-data in loopback mode
void filterMidi() {
  // read RX character
  uint8_t midichar = midi_rx();

  if (isCommand(midichar)) {
    // System-Exclusiv-Ende annehmen
    mSysExActive = false;

    // handle MIDI commands - high-nibble only
    switch (midichar & MIDI_COMMAND_MASK) {
    case MIDI_NOTE_ON:
      midichar = handleNoteOn(midichar);
      break;
    case MIDI_PROGRAM_CHANGE:
      midichar = handleProgramChange(midichar);
      break;
    default:
      if (midichar == MIDI_ACTIVE_SENSING)
        midichar = handleActiveSensing(midichar);
      else if (midichar == MIDI_SYSTEM_EXCLUSIVE)
        mSysExActive = true;
      break;
    }
  }

  midi_tx_filtered(midichar);
}

// forward one midi byte in interface-Mode bidirectional (USB->UART, UART->USB)
void forwardMidi() {
  uint8_t midichar;

  sem_acquire_blocking(&mSemTasks2Exec);

  if (FIFO_available(mFifoUartIn)) {
	  usb_tx(FIFO16_read(mFifoUartIn));
    if (FIFO_available(mFifoUsbIn)) sem_acquire_blocking(&mSemTasks2Exec);
  } 
  
  if (FIFO_available(mFifoUsbIn)) midi_tx(FIFO16_read(mFifoUsbIn));

  tud_task(); // tinyusb device task
}
