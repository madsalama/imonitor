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

// improve: make below #defines 
// configurable instead of hardwired

#define PID_PATH "/var/tmp/imonitor.pid"
#define SOCK_PATH "/tmp/imonitor.socket"
#define LOG_PATH "/var/tmp/imonitord.log"
#define MAX_WATCH 2048

void handle_connection(int);
void handle_request(char* request_buffer, char* response_buffer);

// search for given path in wtable
// if found: return wd and index (where path is found)
int lookup_wd(char path[], int* index);

// search for appropriate location to add new watch_data
// either in an "removed data" location [path->NULL] or at the end of the queue (where: local count < watch_count)
int lookup_adding_index();

void list_watches(char list[]); 
char* watch_list;

void handle_child(int sig);

void handle_args(int argc, char* argv[]);
void kill_daemon();
void daemonize();
void stop_server();
void init_socket();

int server_sockfd;
int fd;

// ~ 5KB/WATCH
struct watch_data{
        int wd;
	char* path;
        // char path[PATH_MAX]; // issue/bug: stack allocated while wtable on heap?
};

struct watch_data* wtable ;
int watch_count;

/* imonitord: unix domain socket inotify daemon
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

        signal(SIGCHLD, handle_child);  // A CHILD RECEIVED A SIG, WHAT SHOULD PARENT DO...?
        signal(SIGTERM, stop_server);

        init_socket();

        if(listen(server_sockfd, 5) == -1)
        {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        printf("Listening...\n");
        fflush(stdout);


	// INITIALIZE INOTIFY INSTANCE
        fd = inotify_init1(IN_NONBLOCK);
        if (fd == -1) {
        perror("inotify_init1");
              exit(EXIT_FAILURE);
        }

/*
	// ALLOCATE MEMORY FOR WATCH DESCRIPTORS
	wd = calloc(MAX_WATCH, sizeof(int));
        if (wd == NULL) {
	        perror("calloc");
		exit(EXIT_FAILURE);
        }
	
	// ALLOCATE MEMORY FOR CORRESPONDING WATHCED PATHS
	paths = calloc(MAX_WATCH, sizeof(PATH_MAX));
	if (paths == NULL){
		perror("calloc");
		exit(EXIT_FAILURE);
	}
*/

	// more elegant way of creating a key/value table
	wtable = calloc( MAX_WATCH, sizeof(struct watch_data) );
	if (wtable == NULL){
	        perror("calloc");
        	exit(EXIT_FAILURE);
	}
	
	watch_list = calloc ( MAX_WATCH * PATH_MAX, sizeof(char) );

        for(;;) // MAIN LOOP
        {
                printf("Waiting for a connection\n");
                fflush(stdout);
                t = sizeof(remote);
	
		// accept() will block until a connection is received
                if((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote, &t)) == -1)
                {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }

                printf("Accepted connection\n");
                fflush(stdout);
                
		handle_connection(client_sockfd);

		// if (poll()){
		//	handle_inotify_events();
		// }

        }
}


void handle_request(char* request_buffer, char* response_buffer){

// request format: '<action_len><path_len><wd><action><path>\0'
// client is responsible to send correct format (errors checked at client)

// deserialize request_buffer

struct request_data rd, *rd_ptr;
rd_ptr=&rd;

deserialize_request_data(request_buffer, rd_ptr);

char action[10];
strcpy(action, rd.action);

int path_len = rd_ptr -> path_len;

char path[path_len];
strcpy(path, rd.path);

if(!strcmp(action,"add")){

	int wd;
	int index;
	
	// LOOKUP IFF MAX_WATCH NOT EXCEEDED
	if ( watch_count < MAX_WATCH ){
		wd = lookup_wd(path, &index); 
	}
	else {
		sprintf(response_buffer, "[ERROR] Max number of %d watches exceeded.\
Remove some watches and try again.", MAX_WATCH);
		return;
	}
	// --------
	
	// ATTEMPT TO ADD WATCH
	if (wd > 0){ // FAIL
		sprintf(response_buffer,"[ERROR] Watch on %s already exists!", path);
	} // FAIL

else {
	int index = lookup_adding_index();
	
	if((wtable[index].wd = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_OPEN | IN_CLOSE_WRITE )) == -1  ){ 
		sprintf(response_buffer, "[ERROR] Could not add watch on %s : %s", path, strerror(errno));
	}
	else
	{ // SUCCESS! 
		wtable[index].path = calloc(path_len, sizeof(char));
		strncpy(wtable[index].path, path, path_len); 
		watch_count++;
		sprintf(response_buffer, "[INFO] Watch added on %s | index: %d", path, index);
	}
     }


}

	else if (!strcmp(action,"remove")){

		// client sends path, map path to corresponding wd
		int index;
		int wd = lookup_wd(path, &index);

		if (wd < 0){
			sprintf(response_buffer, "[ERROR] Watch on %s doesn't exist!", path);
		}
                else if( inotify_rm_watch(fd, wd) == -1  ){
                        sprintf(response_buffer, "[ERROR] Could not remove watch on %d : %s ", wd, strerror(errno));
                }
                else {	
			sprintf(response_buffer, "[INFO] Watch on %s removed on index %d", path, index);
			memset(wtable[index].path,0, path_len);      // clear memory
			free(wtable[index].path);                    // free memory
			wtable[index].path = NULL;                   // nullify pointer
			watch_count--;
                     }
	}
	else if (!strcmp(action,"list") && (watch_count > 0) ){
                
		// emptying list to avoid strcat issues
                // that concats old values for some weird reason O_O'

		memset(watch_list, 0, strlen(watch_list));
		list_watches(watch_list);
		sprintf(response_buffer, "[INFO]: CURRENTLY WATCHING:\n%s", watch_list);
		
	}
	else{
		sprintf(response_buffer,"[ERROR]: No watches exist to list!");
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
		
		// note: request_buffer should contain a serialized request_data struct 
		request_buffer[len] = '\0';
		handle_request( (char*)request_buffer, (char*)response_buffer);   // calls deserialize and handles request

		// after handle_request is returned, response buffer is set and ready to be sent back to client
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
        	free(wtable);
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
        unlink(PID_PATH);  // remove pidfile and socket
        unlink(SOCK_PATH);
        kill(0, SIGKILL);  // kill process children
        
	close(fd);
        free(wtable);
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

	// chmod(local.sun_path, 0777); // enable permissons on socket file

        if(bind(server_sockfd, (struct sockaddr *)&local, len) == -1)
        {
                perror("binding");
                exit(EXIT_FAILURE);
        }
}


// ----------------------------
//  improve: algorithm -> O(N)
// ----------------------------
int lookup_wd(char path[], int* index){
	int i;
	for (i = 0; i < watch_count; i++){
		if ( wtable[i].path != NULL ){
	                if( !strcmp(wtable[i].path, path) ){
                        	*index = i;
	                        return wtable[i].wd;
                	}
		}
	}	
	return -1;
}

int lookup_adding_index(){
	int index = 0;
	int count = 0;
	for (index = 0; count < watch_count; index++){
		if ( wtable[index].path == NULL )
			return index;
		else
			count++; // found one!
	}
	return index;
}

void list_watches(char list[]){
	char string[PATH_MAX]; // iteration variable
	int count = 0;
	int i;

	for (i = 0; count < watch_count; i++){
		if ( wtable[i].path == NULL )
			continue;
		else {
			sprintf(string, "- %s\n", wtable[i].path);
			strcat(list, string);
			count++; // found one!
		}

	// improve: add code to remove trailing \n for final path
	}
}
// -----------------------

