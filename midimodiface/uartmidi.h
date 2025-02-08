#ifndef UARTMIDI_H_
#define UARTMIDI_H_

extern uint8_t midi_rx();
extern void midi_tx(uint8_t);
extern void txMidiString(uint8_t *, int);
extern void uartMidiInit();

extern FIFO16_t mFifoUartIn;

#endif /* UARTMIDI_H_*/
