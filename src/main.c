#include <stdio.h>
#include <string.h>
#include <zlib.h>

int main(int argc, char** argv) {
  printf("%lX\n", crc32(0, (const void*)argv[1], strlen(argv[1])));
  return 0;
}
