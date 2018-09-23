#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

#define LOG_PATH "/var/tmp/monitoring.log"

/* 
   Read all available inotify events from the file descriptor 'fd'.
   wd is the table of watch descriptors for the directories in argv.
   argc is the length of wd and argv.
   argv is the list of watched directories.
   Entry 0 of wd and argv is unused. 
*/
		  
void handle_events(int fd)  {

           char buf[4096]
               __attribute__ ((aligned(__alignof__(struct inotify_event))));

// struct inotify_event {
// int      wd;       /* Watch descriptor */
// uint32_t mask;     /* Mask describing event */
// uint32_t cookie;   /* Unique cookie associating related events (for rename(2)) */
// uint32_t len;      /* Size of name field */
// char     name[];   /* Optional null-terminated name */
//};
	   const struct inotify_event *event;
           
	   int i;
           ssize_t len;
           char *ptr;

           /* Loop while events can be read from inotify file descriptor. */

           for (;;) {

               /* Read some events. */

               len = read(fd, buf, sizeof buf);
               if (len == -1 && errno != EAGAIN) {
                   perror("read");
                   exit(EXIT_FAILURE);
               }

               /* If the nonblocking read() found no events to read, then
                  it returns -1 with errno set to EAGAIN. In that case,
                  we exit the loop. */

               if (len <= 0)
                   break;

               /* Loop over all events in the buffer */

               for (ptr = buf; ptr < buf + len;
                       ptr += sizeof(struct inotify_event) + event->len) {

                   event = (const struct inotify_event *) ptr;

		   // :FILE:
		   // IN_ACCESS? mmap(original):
		   // 		- IN_CLOSE_WRITE? mmap(final) -> sdiff(original, final) -> 
		   // 	flush_diff_to_disk -> munmap(original,final)
		   
		   // WATCH ON DIR
                   if (event->mask & IN_ACCESS){
			printf("IN_ACCESS: ");
			fflush(stdout);

                   if (event->len){
			printf("%s", event->name); // GET FILE NAME
			fflush(stdout);
		   }
	
	           // dir_path = lookup_path_wd(event->wd);
	           // file_path = strcat(dir_path, event->len); 

		   }

               }
           }
       }

// imonitord: init 
// -> fork(): handle_inotify_events(<params>);
int handle_inotify_events(int fd) {

	int out = open(LOG_PATH, O_RDWR|O_CREAT|O_APPEND, 0600);
	if (-1 == out) { perror(LOG_PATH); return 255; }

	if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
        if (-1 == dup2(out, fileno(stderr))) { perror("cannot redirect stderr"); return 255; }

	
        int i, poll_num;
        nfds_t nfds;
        struct pollfd fds[2];

        nfds = 1;

        fds[0].fd = fd;
        fds[0].events = POLLIN;

        printf("imonitord-fork: polling for inotify events...\n");
        
	while (1) {
		poll_num = poll(fds, nfds, -1);
		if (poll_num == -1) {
			if (errno == EINTR)
				continue;
		perror("poll");
		exit(EXIT_FAILURE);
		}

		if (poll_num > 0) {
			if (fds[0].revents & POLLIN) {
				handle_events(fd);
			}
		}
           }

	fflush(stdout); close(out);
        fflush(stderr);

           exit(EXIT_SUCCESS);
}
