#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>


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

                   /* Print event type */

                   if (event->mask & IN_OPEN)
                       printf("IN_OPEN: ");
                   if (event->mask & IN_CLOSE_NOWRITE)
                       printf("IN_CLOSE_NOWRITE: ");
                   if (event->mask & IN_CLOSE_WRITE)
                       printf("IN_CLOSE_WRITE: ");

                   /* Print the name of the watched directory */
	           /*
                   for (i = 1; i < argc; ++i) {
                       if (wd[i] == event->wd) {
                           printf("%s/", argv[i]);
                           break;
                       }
                   }
			*/

                   /* Print the name of the file */

                   if (event->len)
                       printf("%s", event->name);

                   /* Print type of filesystem object */

                   if (event->mask & IN_ISDIR)
                       printf(" [directory]\n");
                   else
                       printf(" [file]\n");
               }
           }
       }

// imonitord: init 
// -> fork(): handle_inotify_events(<params>);
int handle_inotify_events(int fd) {

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

           exit(EXIT_SUCCESS);
}
