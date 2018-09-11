#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/signal.h>
#include <wait.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>
#include <limits.h>

#include "serialize.h"

#define PID_PATH "/var/tmp/imonitor.pid"
#define SOCK_PATH "/tmp/imonitor.socket"
#define LOG_PATH "/var/log/imonitord.log"

#define MAX_WATCH 100 // should be configurable

void handle_connection(int);
void handle_request(char* request_buffer, char* response_buffer);
void handle_child(int sig);
void handle_args(int argc, char* argv[]);
void kill_daemon();
void daemonize();
void stop_server();
void init_socket();

int server_sockfd;
int fd;
int *wd;
int watch_count;

/* imonitord: unix domain server daemon
 * init();
 * a. listens for imonitor requests on /var/run/monitor.socket
 * b. creates an inotify instance to serve upcoming watch requests dynamically

 * handle();
 * c. add/remove watches as per incoming requests
 * d. calls watch handler in a subprocess (fork) -> while(1)/POLL
 */

int main(int argc, char *argv[])
{
        int client_sockfd;
	watch_count = 0;

        struct sockaddr_un remote;
        int t;

        if(argc > 1) {handle_args(argc, argv);}

        signal(SIGCHLD, handle_child);  // A CHILD RECEIVED A SIG, WHAT SHOULD PARENT DO?
        signal(SIGTERM, stop_server);

        init_socket();

        if(listen(server_sockfd, 5) == -1)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        printf("Listening...\n");
        fflush(stdout);


	// INITIALIZE INOTIFY 
        fd = inotify_init1(IN_NONBLOCK);
        if (fd == -1) {
        perror("inotify_init1");
              exit(EXIT_FAILURE);
        }

	// WD
	wd = calloc(MAX_WATCH, sizeof(int));
        if (wd == NULL) {
	        perror("calloc");
		exit(EXIT_FAILURE);
        }

        for(;;) //This might take a while...
        {
                printf("Waiting for a connection\n");
                fflush(stdout);
                t = sizeof(remote);
                if((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote, &t)) == -1)
                {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }

                printf("Accepted connection\n");
                fflush(stdout);
                
		handle_connection(client_sockfd);
        }
}

/*
char *output request_substring(char* input){

}
*/

void handle_request(char* request_buffer, char* response_buffer){

// request format: <action>:<path>:<wd>
// client is responsible to send correct format (error check at client)

// deserialize request_buffer
// struct request_data rd;
// rd = deserialize_request_data(request_buffer);
// char* action = rd.action;
// char* path = rd.path;
// int wd_id  = rd.wd;

char* action = "add";
char* path = "/var/log";
int wd_id = 0;

	if(!strcmp(action,"add")){
		if( (wd[watch_count] = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_OPEN | IN_CLOSE_WRITE )) == -1  ){
                        sprintf(response_buffer, "Could not add watch on %s : %s", path, strerror(errno));
		}
		else {
			++watch_count;
                        sprintf(response_buffer, "[DEBUG]: %s | [INFO] Watch added on %s", request_buffer , path);
                     }
	}

	else if (!strcmp(action,"remove")){

                if( inotify_rm_watch(fd, wd_id) == -1  ){
                        sprintf(response_buffer, "Could not remove watch on %s : %s", path, strerror(errno));
                }
                else {
                        --watch_count;
                        sprintf(response_buffer, "Watch on %s removed", path);
                     }
	}
}

void handle_connection(int client_sockfd)
{
        unsigned char request_buffer[PATH_MAX]; 
	unsigned char response_buffer[PATH_MAX];
        unsigned int len;
	printf("Handling connection\n");
        fflush(stdout);
	
									     // ^ handle potential buffer overflow
        while(len = recv(client_sockfd, &request_buffer, PATH_MAX , 0), (len > 0 && len < PATH_MAX) ){
		request_buffer[len]='\0'; // null-terminate request string
		handle_request( (char*)request_buffer, (char*)response_buffer);
		send(client_sockfd, &response_buffer, PATH_MAX, 0);
	}

        close(client_sockfd);
        printf("Done handling\n");
        fflush(stdout);
}

void handle_child(int sig)
{
        int status;

        printf("Cleaning up child\n");
        fflush(stdout);

        wait(&status);
}


void handle_args(int argc, char *argv[])
{
        if(strcmp(argv[1], "kill") == 0) {kill_daemon();}
        if(strcmp(argv[1], "-D") == 0 || strcmp(argv[1], "--daemon") == 0) {daemonize();}
}

void kill_daemon()
{
        FILE *pidfile;
        pid_t pid;
        char pidtxt[32];

        if(pidfile = fopen(PID_PATH, "r"))
        {
                fscanf(pidfile, "%d", &pid);
                printf("Killing PID %d\n", pid);
                kill(pid, SIGTERM); //kill it gently
	        close(fd);
        	free(wd);
        }
        else
        {
                printf("un_server not running\n"); //or you have bigger problems
        }
	exit(EXIT_SUCCESS);
}

void daemonize()
{
        FILE *pidfile;
        pid_t pid;

        switch(pid = fork())
        {
                case 0:
                        //redirect I/O streams
                        freopen("/dev/null", "r", stdin);
                        freopen(LOG_PATH, "w", stdout);
                        freopen(LOG_PATH, "w", stderr);
                        //make process group leader
                        setsid();
                        chdir("/");
                        break;
                case -1:
                        perror("Failed to fork daemon\n");
                        exit(EXIT_FAILURE);
                default:
                        //write pidfile & you can go home!
                        pidfile = fopen(PID_PATH, "w");
                        fprintf(pidfile, "%d", pid);
                        fflush(stdout);
                        fclose(pidfile);
                        exit(EXIT_SUCCESS);
        }
}

void stop_server()
{
        unlink(PID_PATH);  //Get rid of pesky pidfile, socket
        unlink(SOCK_PATH);
        kill(0, SIGKILL);  //Infanticide :(
        
	close(fd);
        free(wd);
        exit(EXIT_SUCCESS);
}

void init_socket()
{
        struct sockaddr_un local;
        int len;

        if((server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        {
                perror("Error creating server socket");
                exit(EXIT_FAILURE);
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, SOCK_PATH);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);

	chmod(local.sun_path, 0777); // enable permissons on socket file

        if(bind(server_sockfd, (struct sockaddr *)&local, len) == -1)
        {
                perror("binding");
                exit(EXIT_FAILURE);
        }
}
