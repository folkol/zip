/*
 4.3.6 Overall .ZIP file format:

      [local file header 1]
      [encryption header 1]
      [file data 1]
      [data descriptor 1]
      .
      .
      .
      [local file header n]
      [encryption header n]
      [file data n]
      [data descriptor n]
      [archive decryption header]
      [archive extra data record]
      [central directory header 1]
      .
      .
      .
      [central directory header n]
      [zip64 end of central directory record]
      [zip64 end of central directory locator]
      [end of central directory record]
 */

#include <stdio.h>
#include <string.h>
#include <zlib.h>

int main(int argc, char** argv) {
  printf("%lX\n", crc32(0, (const void*)argv[1], strlen(argv[1])));
  return 0;
}
