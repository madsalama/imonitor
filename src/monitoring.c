#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <regex.h>        
#include <string.h>
#include <time.h>

// should be configurable
// create a new file per watch
// append file if exists
// add timestamp for events

#define MONITORING_LOG_PATH "/var/tmp/monitoring.log"

// FILE* stdout; 

/* monitoring.c | module for generating a monitoring report
 * the report lgos events for directory content addition/modification/deletion 
 * also shows if the directory itself was moved
 *
 * the report does not show anything related to hidden files since
 * they are generated by programs like vim and they are usually out of interest
 *
 * this function runs in a separate thread  = this was needed to read the same memory of the parent 
 * since this function needs to show the full path of each event using a lookup operation (wd -> path)
 *
 */



/*
 * Read all available inotify events from the file descriptor 'fd'.
 * wd is the table of watch descriptors for the directories in argv.
 * Entry 0 of wd and argv is unused.
 */

// void handle_events(int fd);
// . . . lookup_path(event->wd); // uses *wtable

struct watch_data{
        int wd;
        char* path;
};

struct thread_data{
	struct watch_data* wtable;
	int fd;
	int* watch_count; 
};

char* lookup_path(int watch_count, int wd);


struct thread_data* tdata ;
struct watch_data* wtable ;
int fd ;
int* thread_watch_count ;

FILE *file;

void timestamp();

void handle_events(int fd)  {

	char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));


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
	regex_t regex;
	int reti;
	char* path;

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

		// CHECK MODIFIED FILES INSIDE WATCHED DIRECTORY
		if ( event->len ) {

			reti = regcomp(&regex, "^[.]", 0);  // ignore hidden files

			if (reti) {
				fprintf(file,"Could not compile regex\n");fflush(file);
			}

			reti = regexec(&regex, event->name, 0, NULL, 0);

			if (!reti) { // HIDDEN FILE MATCHED 
				// fprintf(file,"hidden file = ignoring\n"); fflush(stdout);
			}

			else if (reti == REG_NOMATCH) {

				path = lookup_path(*thread_watch_count, event -> wd );	

				if (event->mask & IN_OPEN ){
					// KEEP TRACK OF OPEN FILES (PATH)
	                                if ( !(event->mask & IN_ISDIR) ) {
						// TRIGGERED TWICE FOR SOME REASON!?
                                                // fprintf(file,"IN_OPEN %s/%s\n", path, event->name );fflush(file);
                                                // OPEN/READ FILE 
                                        }
				}

				else if ( event->mask & IN_CREATE ) {
					timestamp();
                                	if ( event->mask & IN_ISDIR ) {
	                                        fprintf(file,"+DIR: %s/%s\n\n", path, event->name );fflush(file);
	                                }
	                                else {
	                                        fprintf(file,"+FILE: %s/%s\n\n", path, event->name );fflush(file);
	                                }
	                        }
	                        else if ( event->mask & IN_DELETE ) {
					timestamp();
	                                if ( event->mask & IN_ISDIR ) {
	                                        fprintf(file,"-DIR: %s/%s\n\n", path, event->name );fflush(file);
	                                }
	                                else {
	                                        fprintf(file,"-FILE: %s/%s\n\n", path, event->name );fflush(file);
	                                }
	                        }
				
	                        else if ( event->mask & IN_MODIFY ) {
	                                if ( event->mask & IN_ISDIR ) {
	                                        // fprintf(file,"M DIR: %s/%s\n",path, event->name );fflush(file);
	                                }
	                                else {
	                                        // fprintf(file,"M FILE: %s/%s\n",path, event->name );fflush(file);
	                                }
	                        }
				

				else if ( event->mask & IN_CLOSE_WRITE ) { // FILE MODIFIED
					if (event->mask & IN_ISDIR) {
						// TRIGGERED MANY TIMES -> TEMPORARY FILES (READ/ADD/DELETE...)
                                     	fprintf(file,"DIR: IN_CLOSE_WRITE %s/%s\n\n",path, event->name );fflush(file);
                                        }
					else
					fprintf(file,"FILE: IN_CLOSE_WRITE %s/%s\n\n",path, event->name );fflush(file);
				}
				free(path);
			}
			else {
				// regerror(reti, &regex, (const)event->name, event->len);
				fprintf(file,"Regex match failed: %s/%s\n",path, event->name);fflush(file);
			}

			regfree(&regex);
		}
	}
	}
}



// imonitord: init 
// -> fork(): handle_inotify_events(<params>);

void* handle_inotify_events(void* args) {

	// GET ARGS FROM MAIN PROCESS
	tdata = args;
	wtable = tdata -> wtable;
	fd = tdata -> fd;
	thread_watch_count = tdata -> watch_count;  

	file = fopen(MONITORING_LOG_PATH, "a");
	if (file == NULL){
		perror("fopen");
		exit(EXIT_FAILURE);
	}

        int i, poll_num;
        nfds_t nfds;
        struct pollfd fds[2];

        nfds = 1;

        fds[0].fd = fd;
        fds[0].events = POLLIN;
        
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

	fclose(file);
        exit(EXIT_SUCCESS);
}



char* lookup_path(int watch_count, int wd){

	// linear lookup O(N)
	
        int i;
        int count = 0 ;
        for (i = 0; count < watch_count; i++){
                if ( wtable[i].wd == wd ){
			char* path = calloc(strlen(wtable[i].path)+1, sizeof(char));
			strcpy(path, wtable[i].path);
			return path;  
		}
		
                else {
			count++;
			continue;
		}
	}
	return NULL; 
}

void timestamp()
{
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    fprintf(file, "%s", asctime( localtime(&ltime) ) );
}


