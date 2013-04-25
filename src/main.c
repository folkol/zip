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

const long END_OF_DIR_SIGNATURE = 0x06054b50;
const long DIR_HEADER_SIGNATURE = 0x02014b50;
const long FILE_HEADER_SIGNATURE = 0x04034b50;

typedef struct {
  short version_needed;              // version needed to extract       2 bytes
  short bit_flag;                    // general purpose bit flag        2 bytes
  short compression_method;          // compression method              2 bytes
  short last_modified_time;          // last mod file time              2 bytes
  short last_modified_date;          // last mod file date              2 bytes
  long  checksum;                    // crc-32                          4 bytes
  long  compressed_size;             // compressed size                 4 bytes
  long  uncompressed_size;           // uncompressed size               4 bytes
  short filename_length;             // file name length                2 bytes
  short extra_field_length;          // extra field length              2 bytes
  char* filename;                    // file name (variable size)
  char* extra_field;                 // extra field (variable size)
} local_file_header;

typedef struct {
} encryption_header;

typedef struct {
  short version_made_by;             // version made by                 2 bytes
  short version_needed;              // version needed to extract       2 bytes
  short bit_flag;                    // general purpose bit flag        2 bytes
  short compression_method;          // compression method              2 bytes
  short last_modified_time;          // last mod file time              2 bytes
  short last_modified_date;          // last mod file date              2 bytes
  long  checksum;                    // crc-32                          4 bytes
  long  compressed_size;             // compressed size                 4 bytes
  long  uncompressed_size;           // uncompressed size               4 bytes
  short filename_length;             // file name length                2 bytes
  short extra_field_length;          // extra field length              2 bytes
  short comment_length;              // file comment length             2 bytes
  short disk_no;                     // disk number start               2 bytes
  short internal_file_attr;          // internal file attributes        2 bytes
  long  external_file_attr;          // external file attributes        4 bytes
  long  rel_offset_local_header;     // relative offset of local header 4 bytes
  char* filename;                    // file name (variable size)
  char* extra_field;                 // extra field (variable size)
  char* comment;                     // file comment (variable size)
} dir_header;

dir_header* dir;

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

void read_dir() {
  int x;
  long signature = 0;
  dir = (dir_header*) malloc(end_of_dir.dir_entries * sizeof(dir_header));
  fseek(file, end_of_dir.dir_offset, SEEK_SET);
  for(x = 0; x < end_of_dir.dir_entries; x++) {
    fread(&signature, 4, 1, file);
    if(signature != DIR_HEADER_SIGNATURE) {
      fprintf(stderr, "Wrong signature for directory header!");
      exit(EXIT_FAILURE);
    } else {
      printf("Found file header");
      fread(&dir[x].version_made_by, 2, 1, file);
      fread(&dir[x].version_needed, 2, 1, file);
      fread(&dir[x].bit_flag, 2, 1, file);
      fread(&dir[x].compression_method, 2, 1, file);
      fread(&dir[x].last_modified_time, 2, 1, file);
      fread(&dir[x].last_modified_date, 2, 1, file);
      fread(&dir[x].checksum, 4, 1, file);
      fread(&dir[x].compressed_size, 4, 1, file);
      fread(&dir[x].uncompressed_size, 4, 1, file);
      fread(&dir[x].filename_length, 2, 1, file);
      fread(&dir[x].extra_field_length, 2, 1, file);
      fread(&dir[x].comment_length, 2, 1, file);
      fread(&dir[x].disk_no, 2, 1, file);
      fread(&dir[x].internal_file_attr, 2, 1, file);
      fread(&dir[x].external_file_attr, 4, 1, file);
      fread(&dir[x].rel_offset_local_header, 4, 1, file);

      dir[x].filename = (char*) malloc(dir[x].filename_length + 1);
      fread(dir[x].filename, dir[x].filename_length, 1, file);
      dir[x].filename[dir[x].filename_length] = '\0';

      dir[x].extra_field = (char*) malloc(dir[x].extra_field_length + 1);
      fread(dir[x].extra_field, dir[x].extra_field_length, 1, file);
      dir[x].extra_field[dir[x].extra_field_length] = '\0';

      dir[x].comment = (char*) malloc(dir[x].comment_length + 1);
      fread(dir[x].comment, dir[x].comment_length, 1, file);
      dir[x].comment[dir[x].comment_length] = '\0';

      printf("Filename: %s\n", dir[x].filename);
      printf("Extra field: %s\n", dir[x].extra_field);
      printf("Comment: %s\n", dir[x].comment);
      printf("encrypted: %d\n", dir[x].bit_flag & 0x1);
    }
  }
}

void read_end_of_dir() {
  long offset = -4;
  long signature = 0;
  while(1) {
    if(fseek(file, offset, SEEK_END)) {
      fprintf(stderr, "Zip file is corrupt!");
      exit(EXIT_FAILURE);
    }
    fread(&signature, 4, 1, file);
    if(signature == END_OF_DIR_SIGNATURE) {
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

void extract_file(const dir_header dir_entry) {
  long signature = 0;
  local_file_header h;
  printf("About to extract file: %s\n", dir_entry.filename);
  fseek(file, dir_entry.rel_offset_local_header, SEEK_SET);
  fread(&signature, 4, 1, file);
  if(signature != FILE_HEADER_SIGNATURE) {
    fprintf(stderr, "Wrong signature for local file header, was %lx!", signature);
    exit(EXIT_FAILURE);
  } else {
      printf("Found file header");
      fread(&h.version_needed, 2, 1, file);
      fread(&h.bit_flag, 2, 1, file);
      fread(&h.compression_method, 2, 1, file);
      fread(&h.last_modified_time, 2, 1, file);
      fread(&h.last_modified_date, 2, 1, file);
      fread(&h.checksum, 4, 1, file);
      fread(&h.compressed_size, 4, 1, file);
      fread(&h.uncompressed_size, 4, 1, file);
      fread(&h.filename_length, 2, 1, file);
      fread(&h.extra_field_length, 2, 1, file);

      h.filename = (char*) malloc(h.filename_length + 1);
      fread(h.filename, h.filename_length, 1, file);
      h.filename[h.filename_length] = '\0';

      h.extra_field = (char*) malloc(h.extra_field_length + 1);
      fread(h.extra_field, h.extra_field_length, 1, file);
      h.extra_field[h.extra_field_length] = '\0';

      printf("Filename: %s\n", h.filename);
      printf("Extra field: %s\n", h.extra_field);
      printf("Compression method: %d\n", h.compression_method);

      if(h.bit_flag & 0x1) {
        fprintf(stderr, "fzip does not support encypted zip files, sry :S");
        exit(EXIT_FAILURE);
      }
  }
}

int main(int argc, char** argv) {
  char* filename = argv[1];
  if(argc < 2) {
    print_usage();
  }

  file = fopen(argv[argc - 1], "rb");
  if (!file) {
    fprintf(stderr, "Unable to open file %s", filename);
    return;
  }

  read_end_of_dir();
  read_dir();

  int x;
  for(x = 0; x < end_of_dir.dir_entries; x++) {
    extract_file(dir[x]);
  }

  fclose(file);

  //  printf("%lX\n", crc32(0, (const void*)argv[1], strlen(argv[1])));
  return 0;
}
