#include <stdio.h>
#include <stdlib.h>
#include "mmap_file.h"

int main(int argc, char* argv[]){

	if (argc<2){
		printf("NO ARGS");
		exit(EXIT_FAILURE);
	}

	char *path;
	struct fmeta m;
	char* p;
	off_t len;

	path = argv[1];

	m = mmap_file(path);
	p = m.fptr;

        for (len = 0; len < m.flen; len++)
                putchar (p[len]);

        if (munmap (m, m.flen) == -1) {
                perror ("munmap");
                return 1;
        }
}
