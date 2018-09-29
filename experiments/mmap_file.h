#ifndef MMAP_FILE_H
#define MMAP_FILE_H

struct fmeta{
	char *fptr;
	off_t flen;
};

struct fmeta mmap_file(char* path);

#endif /* MMAP_FILE_H */
