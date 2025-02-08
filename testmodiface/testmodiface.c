#include <assert.h>

#include "bsp/board_api.h"
#include "pico/stdlib.h"

#include "fifo.h"
#include "midiprocessor.h"


// MIDI-Defindes
#define NOTE_OFF         0x80
#define NOTE_ON          0x90
#define POLY_PRESSURE    0xA0
#define CONTROL_CHANGE   0xB0
#define PROGRAM_CHANGE   0xC0
#define MONO_PRESSURE    0xD0
#define PITCH_BEND       0xE0
#define SYSTEM_EXCLUSIVE 0xF0
#define SYSTEM_EX_END    0xF7
#define ACTIVE_SENSING	 0xFE

semaphore_t mSemTasks2Exec;

// swich operation mode to loopback (incl. velocity processing)
void loopbackOn() {
}

// swich operation mode to MIDI-interface (forwarding in both directions)
void loopbackOff() {
}

unsigned char testIn[] = {ACTIVE_SENSING,
						  POLY_PRESSURE,0x42,0x43,
						  CONTROL_CHANGE,27,54,
						  PROGRAM_CHANGE,0x7F,
						  NOTE_ON,0x42,0x63,NOTE_OFF,0x42,0x63,
						  ACTIVE_SENSING,
						  ACTIVE_SENSING,
						  MONO_PRESSURE,0x63,
						  PROGRAM_CHANGE,23,
						  NOTE_ON,0x52,0x22,NOTE_OFF,0x52,0x63,
						  PITCH_BEND,0x42,33,
						  SYSTEM_EXCLUSIVE,0,1,2,3,4,SYSTEM_EX_END,NOTE_ON,0x42,0,
						  SYSTEM_EXCLUSIVE,0,1,2,3,4,PROGRAM_CHANGE,0x42,
						  NOTE_ON,0x52,0x22,NOTE_OFF,0x52,0x63,
						  PROGRAM_CHANGE,22,
						  NOTE_ON,0x37,0x23,
						  PROGRAM_CHANGE,0,
						  NOTE_ON,0x38,0x24,
						  PROGRAM_CHANGE,21,
						  NOTE_ON,0x39,0x25,
						  PROGRAM_CHANGE,1,
						  NOTE_ON,0x3a,0x26,
						  PROGRAM_CHANGE,12,
						  ACTIVE_SENSING,
						  NOTE_ON,0x3b,0x27,
						  ACTIVE_SENSING,
						  PROGRAM_CHANGE,2,
						  NOTE_ON,0x3c,0x28};

unsigned char testOut[] = {CONTROL_CHANGE,122,0,ACTIVE_SENSING,
						   POLY_PRESSURE,0x42,0x43,
						   CONTROL_CHANGE,27,54,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,0x7F,
						   NOTE_ON,0x42,0x63,NOTE_OFF,0x42,0x63,
						   ACTIVE_SENSING,
						   ACTIVE_SENSING,
						   MONO_PRESSURE,0x63,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,23,
						   NOTE_ON,0x52,0x7F,NOTE_OFF,0x52,0x63,
						   PITCH_BEND,0x42,33,
						   NOTE_ON,0x42,0,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,0x42,
						   NOTE_ON,0x52,0x22,NOTE_OFF,0x52,0x63,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,22,
						   NOTE_ON,0x37,0x7F,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,0,
						   NOTE_ON,0x38,0x24,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,21,
						   NOTE_ON,0x39,0x7F,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,1,
						   NOTE_ON,0x3a,0x26,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,12,
						   ACTIVE_SENSING,
						   NOTE_ON,0x3b,0x7F,
						   ACTIVE_SENSING,
						   CONTROL_CHANGE,122,0,PROGRAM_CHANGE,2,
						   NOTE_ON,0x3c,0x28};

bool testLed[] = {false,false,false,true,
				  true,true,true,
				  true,true,true,
				  true,true,true,true,true,
				  false,false,false,false,false,false,
				  true,
				  false,
				  false,false,
				  false,false,false,false,true,
				  true,true,true,true,true,true,
				  true,true,true,
				  true,true,true,
				  true,true,true,true,false,
				  false,false,false,false,false,false,
				  false,false,false,false,true,
				  true,true,true,
				  true,true,true,true,false,
				  false,false,false,
				  false,false,false,false,true,
				  true,true,true,
				  true,true,true,true,false,
				  false,false,false,
				  false,false,false,false,true,
				  true,
				  true,true,true,
				  true,
				  true,true,true,true,false,
				  false,false,false};


int inPosition = 0;
int outPosition = 0;
bool ledState = false;


// function - get a midi character from UART - blocking
unsigned char midi_rx() {
	unsigned char midichar = testIn[inPosition++];
	return midichar;
}

// function - send a midi character to UART - blocking
void midi_tx(unsigned char _midichar) {
	if ((testLed[outPosition] != ledState) || (testOut[outPosition++] != _midichar))
		while(true) continue; // Test failed
	if (sizeof(testOut) == outPosition)
		while(true) continue; // Test passed

	return;
}

void setLed(bool _state) {
	gpio_put(PICO_DEFAULT_LED_PIN, _state);
	ledState = _state;
	sleep_ms(300);
}

// transmit a string of MIDI-bytes via UART-interface
void txMidiString(uint8_t *_string, int _size) {
  for (int i = 0; i < _size; i++) {
    midi_tx(_string[i]);
  }
}

void tud_task() {}

int main() {
  // initialize Semaphores
  sem_init(&mSemTasks2Exec, 0, 2*FIFO16_SIZE+1);

  // initialize LED
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  // Compile time check that test-output-arrays are of same size
  static_assert(sizeof(testLed) == sizeof(testOut), "Error in Test setup");

  // processing-loop
  while (true) {
    filterMidi();
  }
}