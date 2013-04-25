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

const long DIR_SIGNATURE = 0x06054b50;

struct {
  short disk_no;                     // number of this disk (2 bytes)
  short dir_disk_no;                 // number of the disk with the start of the central directory (2 bytes)
  short dir_entries_this_disk;       // total number of entries in the central directory on this disk (2 bytes)
  short dir_entries;                 // total number of entries in the central directory (2 bytes)
  long  dir_size;                    // size of the central directory (4 bytes)
  long  dir_offset;                  // offset of start of central directory with respect to the starting disk number (4 bytes)
  short comment_length;              // .ZIP file comment length (2 bytes)
  char* comment;                     // .ZIP file comment (variable size)
} end_of_dir;

char* archive_comment;

void print_usage() {
  fprintf(stderr, "usage: fzip filename\n");
  exit(EXIT_FAILURE);
}

void read_end_of_dir() {
  long offset = -4;
  long signature;
  while(1) {
    if(fseek(file, offset, SEEK_END)) {
      fprintf(stderr, "Zip file is corrupt!");
      exit(EXIT_FAILURE);
    }
    fread(&signature, 4, 1, file);
    if(signature == DIR_SIGNATURE) {
      printf("Found end of directory at %li!\n", ftell(file));

      fread(&end_of_dir.disk_no, 2, 1, file);
      fread(&end_of_dir.dir_disk_no, 2, 1, file);
      fread(&end_of_dir.dir_entries_this_disk, 2, 1, file);
      fread(&end_of_dir.dir_entries, 2, 1, file);
      fread(&end_of_dir.dir_size, 4, 1, file);
      fread(&end_of_dir.dir_offset, 4, 1, file);
      fread(&end_of_dir.comment_length, 2, 1, file);
      archive_comment = (char*) malloc(end_of_dir.comment_length + 1);
      fread(archive_comment, end_of_dir.comment_length, 1, file);
      archive_comment[end_of_dir.comment_length] = '\0';
      end_of_dir.comment = archive_comment;

      printf("End of directory structure:\n");
      printf("disk_no: %hd\n", end_of_dir.disk_no);
      printf("dir_disk_no: %hd\n", end_of_dir.dir_disk_no);
      printf("dir_entries_this_disk: %hd\n", end_of_dir.dir_entries_this_disk);
      printf("dir_entries: %hd\n", end_of_dir.dir_entries);
      printf("dir_size: %ld\n", end_of_dir.dir_size);
      printf("dir_offset: %ld\n", end_of_dir.dir_offset);
      printf("comment_length: %hd\n", end_of_dir.comment_length);
      printf("comment: \"%s\".\n", end_of_dir.comment);

      break;
    }
    offset--;
  }
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

  read_end_of_dir();

  fclose(file);

  //  printf("%lX\n", crc32(0, (const void*)argv[1], strlen(argv[1])));
  return 0;
}
