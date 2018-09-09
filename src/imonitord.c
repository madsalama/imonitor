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

#include "mmap_file.h"

#define PID_PATH "/var/tmp/imonitor.pid"
#define SOCK_PATH "/var/run/imonitor.socket"
#define LOG_PATH "/var/log/imonitord.log"

void fork_handler(int);
void handle_connection(int);
void handle_child(int sig);
void handle_args(int argc, char *argv[]);
void kill_daemon();
void daemonize();
void stop_server();
void init_socket();

int server_sockfd;

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
        struct sockaddr_un remote;
        int t;

        if(argc > 1) {handle_args(argc, argv);}

        signal(SIGCHLD, handle_child);
        signal(SIGTERM, stop_server);

        init_socket();

        if(listen(server_sockfd, 5) == -1)
        {
                perror("listen");
                exit(1);
        }

        printf("Listening...\n");
        fflush(stdout);

        for(;;) //This might take a while...
        {
                printf("Waiting for a connection\n");
                fflush(stdout);
                t = sizeof(remote);
                if((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote, &t)) == -1)
                {
                        perror("accept");
                        exit(1);
                }

                printf("Accepted connection\n");
                fflush(stdout);
                //Yes, yes, forks = overhead, blahblah. If you need a high volume local
                //echo server (Ô_o) this ain't it.
                fork_handler(client_sockfd); 
        }
}

void fork_handler(int client_sockfd)
{
        int status;

        switch(fork())
        {
                case -1:
                        perror("Error forking connection handler");
                        break;
                case 0:
                        handle_connection(client_sockfd);
                        exit(0);
                default:
                        break;
        }
}

void handle_connection(int client_sockfd)
{
        char buff[1024];
        unsigned int len;

	printf("Handling connection\n");
        fflush(stdout);

        while(len = recv(client_sockfd, &buff, 1024, 0), len > 0)
	{
		// check received request
		// if list -> list wd
		// if add [path] -> add wd
		// if remove [path/wd] -> remove wd
		
		buff[len]='\0';
		if (!strcmp(&buff,"list")){
			// handle "list"
			// result = handle();
			
			// notify client with result
			send(client_sockfd, "list handled!", 100, 0);	
		}

		// else
		// buff=add:[PATH]
		// EXTRACT ACTION/PATH:
			// ACTION = add
			// PATH = PATH
		
		// $ACTION watch for $PATH

	}

        close(client_sockfd);
        printf("Done handling\n");
        fflush(stdout);
        exit(0);
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
        }
        else
        {
                printf("un_server not running\n"); //or you have bigger problems
        }
        exit(0);
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
                        exit(1);
                default:
                        //write pidfile & you can go home!
                        pidfile = fopen(PID_PATH, "w");
                        fprintf(pidfile, "%d", pid);
                        fflush(stdout);
                        fclose(pidfile);
                        exit(0);
        }
}

void stop_server()
{
        unlink(PID_PATH);  //Get rid of pesky pidfile, socket
        unlink(SOCK_PATH);
        kill(0, SIGKILL);  //Infanticide :(
        exit(0);
}

void init_socket()
{
        struct sockaddr_un local;
        int len;

        if((server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        {
                perror("Error creating server socket");
                exit(1);
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, SOCK_PATH);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);
        if(bind(server_sockfd, (struct sockaddr *)&local, len) == -1)
        {
                perror("binding");
                exit(1);
        }
}
