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
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

FILE *file;

struct {
  long  signature;                  // end of central dir signature    4 bytes  (0x06054b50)
  short disk_no;                    // number of this disk             2 bytes
  short dir_disk_no;                // number of the disk with the start of the central directory  2 bytes
  short dir_entries_this_disk;      // total number of entries in the central directory on this disk  2 bytes
  short dir_entries;                // total number of entries in the central directory           2 bytes
  long  dir_size;                   // size of the central directory   4 bytes
  long  dir_offset;                 // offset of start of central directory with respect to the starting disk number        4 bytes
  short comment_length;             // .ZIP file comment length        2 bytes
  char* comment;                    // .ZIP file comment       (variable size)
} end_of_directory;

void print_usage() {
  fprintf(stderr, "usage: fzip filename\n");
  exit(EXIT_FAILURE);
}

void read_end_of_directory() {
  long offset = -4;
  long signature;
  while(1) {
    if(fseek(file, offset, SEEK_END)) {
      fprintf(stderr, "Zip file is corrupt!");
      exit(EXIT_FAILURE);
    }
    fread(&signature, 4, 1, file);
    if(signature == 0x06054b50) {
      printf("Found end of directory at %li!", ftell(file));
      exit(EXIT_SUCCESS);
    }
    offset--;
  }
  fprintf(stderr, "Could not find end of directory...");
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  char* filename = argv[1];
  if(argc < 2) {
    print_usage();
  }

  file = fopen(argv[1], "rb");
  if (!file) {
    fprintf(stderr, "Unable to open file %s", filename);
    return;
  }

  read_end_of_directory();

  fclose(file);

  //  printf("%lX\n", crc32(0, (const void*)argv[1], strlen(argv[1])));
  return 0;
}
