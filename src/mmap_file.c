#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "mmap_file.h"

struct fmeta mmap_file (char* path){
	
	struct stat sb;    // fstat struct
	struct fmeta meta; // mmaped file meta data struct

	off_t len;
	char *p;
	int fd;

	fd = open (path, O_RDONLY);

	if (fd == -1) {
		perror ("open");
		exit(EXIT_FAILURE);
	}
	if (fstat (fd, &sb) == -1) {
		perror ("fstat");
		exit(EXIT_FAILURE);
	}

	if (!S_ISREG (sb.st_mode)) {
		fprintf (stderr, "%s is not a file\n", path);
		exit(EXIT_FAILURE);
	}
	
	p = mmap (0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if (p == MAP_FAILED) {
		perror ("mmap");
		exit(EXIT_FAILURE);
	}
	if (close (fd) == -1) {
		perror ("close");
		exit(EXIT_FAILURE);
	}

	meta.fptr = p;
	meta.flen = sb.st_size;


	return meta;

}
