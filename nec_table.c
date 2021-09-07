#include "stdio.h"


int nec_format(unsigned char b) {
  // Reverse bits as NEC sends LSB first
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

  // Calculate inverse in lower bytes
  return (b << 8) + (~b & 0xff);
}


int main(int argc, char **argv) {
    printf("static const int nec_code_table[256] = {\n    ");
    for (unsigned int i=0; i <= 0xff; i++) {
        printf("0x%.4x", nec_format(i));

        if (i != 0xff) {
            printf(", ");
        }

        if ((i % 8 == 7)) {
            printf("\n");
            if (i != 0xff) {
              printf("    ");
            }
        }
    }
    printf("};\n");

    return 0;
}
