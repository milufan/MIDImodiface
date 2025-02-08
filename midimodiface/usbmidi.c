#include "bsp/board_api.h"
#include "pico/stdlib.h"

#include "fifo.h"
#include "usbmidi.h"
#include "midimodiface.h"

FIFO16_t mFifoUsbIn;

// send a midi cbyte to USB - blocking
void usb_tx(uint8_t _midibyte) {
  uint8_t outBuff[1] = {_midibyte};
  tud_midi_stream_write(0, outBuff, 1);
}

// invoked when device is mounted
void tud_mount_cb(void) { loopbackOff(); }

// invoked when device is unmounted
void tud_umount_cb(void) { loopbackOn(); }

// invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) { loopbackOn(); }

// invoked when usb bus is resumed
void tud_resume_cb(void) {
  if (tud_mounted()) loopbackOff();
  else               loopbackOn();
}

// invoked on usb data rx
void tud_midi_rx_cb(uint8_t _itf) {
  uint8_t midichar;

  // copy USB-data to receive-queue
  while (tud_midi_available()) {
    tud_midi_stream_read(&midichar, 1);
    FIFO16_write(mFifoUsbIn, midichar); 
    sem_release(&mSemTasks2Exec);
  }
}

// invoked on USB-interrupts, cause invocation of tud_task
static void __isr USB_IRQHandler(void) { sem_release(&mSemTasks2Exec); }

void usbMidiInit() {
  // Initialize FIFO
  FIFO_init(mFifoUsbIn);

  // initialize tinyUSB
  tud_init(0);
  if (board_init_after_tusb) board_init_after_tusb();

  // setup USB-Interrupts
  irq_add_shared_handler(USBCTRL_IRQ, USB_IRQHandler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
}
