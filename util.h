#ifndef UTIL_H

#define UTIL_H

int nec_format(unsigned char b) {
  // Reverse bits as NEC sends LSB first
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

  // Calculate inverse in lower bytes
  return (b << 8) + (~b & 0xff);
}

#endif // UTIL_H
