#ifndef FIFO_H_
#define FIFO_H_

#define FIFO16_SIZE 16
#define FIFO32_SIZE 32
#define FIFO64_SIZE 64
#define FIFO128_SIZE 128

typedef struct {
  uint8_t _read;
  uint8_t _write;
  uint8_t _buffer[FIFO16_SIZE];
} FIFO16_t;

typedef struct {
  uint8_t _read;
  uint8_t _write;
  uint8_t _buffer[FIFO32_SIZE];
} FIFO32_t;

typedef struct {
  uint8_t _read;
  uint8_t _write;
  uint8_t _buffer[FIFO64_SIZE];
} FIFO64_t;

typedef struct {
  uint8_t _read;
  uint8_t _write;
  uint8_t _buffer[FIFO128_SIZE];
} FIFO128_t;

#define FIFO_init(fifo)                                                        \
  {                                                                            \
    fifo._read = 0;                                                            \
    fifo._write = 0;                                                           \
  }

#define FIFO_available(fifo) (fifo._read != fifo._write)

#define FIFO_read(fifo, size)                                                  \
  ((FIFO_available(fifo))                                                      \
       ? fifo._buffer[fifo._read = (fifo._read + 1) & (size - 1)]              \
       : 0)

#define FIFO_write(fifo, data, size)                                           \
  {                                                                            \
    uint8_t tmphead =                                                          \
        (fifo._write + 1) & (size - 1); /* calculate buffer index */           \
    if (tmphead != fifo._read) {        /* if buffer is not full */            \
      fifo._buffer[tmphead] = data;     /* store data in buffer */             \
      fifo._write = tmphead;            /* store new index */                  \
    }                                                                          \
  }

#define FIFO16_read(fifo) FIFO_read(fifo, FIFO16_SIZE)
#define FIFO16_write(fifo, data) FIFO_write(fifo, data, FIFO16_SIZE)

#define FIFO32_read(fifo) FIFO_read(fifo, FIFO16_SIZE)
#define FIFO32_write(fifo, data) FIFO_write(fifo, data, FIFO16_SIZE)

#define FIFO64_read(fifo) FIFO_read(fifo, FIFO16_SIZE)
#define FIFO64_write(fifo, data) FIFO_write(fifo, data, FIFO16_SIZE)

#define FIFO128_read(fifo) FIFO_read(fifo, FIFO16_SIZE)
#define FIFO128_write(fifo, data) FIFO_write(fifo, data, FIFO16_SIZE)

#endif /*FIFO_H_*/