#ifndef UTIL_H

#define UTIL_H
int nec_format(unsigned char b) {
      // Just need the inverse now
      return (~b << 8) + (b & 0xff);
}

#endif // UTIL_H
