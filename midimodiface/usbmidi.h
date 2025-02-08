#ifndef USBMIDI_H_
#define USBMIDI_H_

extern void usb_tx(uint8_t _midibyte);
extern void usbMidiInit();

extern FIFO16_t mFifoUsbIn;

#endif /* USBMIDI_H_*/
