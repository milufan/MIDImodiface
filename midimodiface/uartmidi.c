#include "bsp/board_api.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "fifo.h"

#include "uartmidi.h"
#include "midimodiface.h"

// UART defines
// By default the stdout UART is `uart0`
#define UART_ID uart1
#define BAUD_RATE 31250
// Use pins 4 and 5 for UART1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_IRQ UART1_IRQ

FIFO16_t mFifoUartIn;

// get a midi byte - blocking
uint8_t midi_rx() {
  while (!FIFO_available(mFifoUartIn)) {
    sem_acquire_blocking(&mSemTasks2Exec);
    tud_task(); // tinyusb device task
  }

  return FIFO16_read(mFifoUartIn);
}

// send a midi byte to UART - blocking
void midi_tx(uint8_t _midibyte) {
  uart_tx_wait_blocking(UART_ID);
  uart_putc(UART_ID, _midibyte);
}

// transmit a string of MIDI-bytes via UART-interface
void txMidiString(uint8_t *_string, int _size) {
  for (int i = 0; i < _size; i++) {
    midi_tx(_string[i]);
  }
}

// invoked on UART-interrupts
static void __isr uart_callback() {
  // copy UART-data to receive-queue
  if (uart_is_readable(UART_ID)) {
    FIFO16_write(mFifoUartIn, uart_getc(UART_ID)); 
    sem_release(&mSemTasks2Exec);
  }
}

void uartMidiInit() {
  // Initialize FIFO
  FIFO_init(mFifoUartIn);

  // Set up UART and speed.
  uart_init(UART_ID, BAUD_RATE);

  // Set the TX and RX pins by using the function select on the GPIO
  // See datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
  gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

  // enable Tx and Rx fifos on UART
  uart_set_fifo_enabled(UART_ID, true);

  // disable cr/lf conversion on Tx
  uart_set_translate_crlf(UART_ID, false);

  // setup UART-Interrupts
  irq_set_exclusive_handler(UART_IRQ, uart_callback);
  irq_set_enabled(UART_IRQ, true);
  uart_set_irq_enables(UART_ID, true, false);
}
