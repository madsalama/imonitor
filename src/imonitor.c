#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "serialize.h"

#define SOCK_PATH "/tmp/imonitor.socket"

// imonitor.c | unix socket client for managing imonitor daemon = (daemon.c)
// USAGE: monitor [add|remove|list|help]

void check_arg(int argc, char* arg);
void synopsis();
void help();

int main(int argc, char *argv[])
{

	if (argc == 1){
      	   printf("imonitor: no arguments. [hint: imonitor help]\n");
	   exit(EXIT_FAILURE);
	}

	check_arg(argc, argv[1]);

        int sockfd, t, len;
        struct sockaddr_un remote;

	unsigned char request_buffer[PATH_MAX], *ptr;
	char response_buffer[PATH_MAX];
	struct request_data rd, *rd_ptr;
	rd_ptr=&rd;
        
	if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        {
                perror("Socket");
                exit(EXIT_FAILURE);
        }

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, SOCK_PATH);
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);

        if(connect(sockfd, (struct sockaddr *)&remote, len) == -1)
        {
		// perror("Connect");
		printf("imonitor: error connecting to daemon at unix://%s\n", SOCK_PATH);
		printf("imonitor: run 'systemctl start imonitord' then retry\n");
                exit(EXIT_FAILURE);
        }

	// BUILD STRUCT REQUEST DATA
	// IN EMPTY CASES WE AVOID SENDING NULL POINTERS BY ADDING DUMMY VALUE
	
	if (strcmp(argv[1],"list")){
		rd.action = argv[1];
		rd.path = argv[2];
		rd.wd = 255; // (int)strtol(argv[3],(char **)NULL,1024); 
		rd.action_len = strlen(argv[1]);
		rd.path_len   = strlen(argv[2]);
	}
	else{
		rd.action = "list";
		rd.path = "x";
		rd.wd = 255;
		rd.action_len = 5; 
		rd.path_len = 5; 
	}

        // SERIALIZE STRUCT -> request_buffer
        // request becomes: <action_len><path_len><action><path><wd>
        // request_buffer's first 2*ints = 2*4 bytes = 8 bytes -> are now holding lengths

	ptr = serialize_request_data(request_buffer, rd_ptr);
	
	/* request_buffer[ptr - request_buffer] = '\0'; */
	
	struct request_data rdd, *rdd_ptr;
	rdd_ptr=&rdd;

	deserialize_request_data(request_buffer, rdd_ptr);
	
        printf("deserialize: %s | %s | %d | %d | %d \n", rdd.action, rdd.path, rdd.wd, rdd.action_len, rdd.path_len ); 
	exit(1);

        if(send(sockfd, request_buffer, ptr - request_buffer , 0) == -1)
        {
            perror("Send");
            exit(EXIT_FAILURE);
        }

        if((t = recv(sockfd, &response_buffer, PATH_MAX, 0)) > 0)
        {
            printf("imonitord: %s \n", response_buffer);
        }
        else
         {
            if(t < 0) perror("recv");
            else printf("Server closed connection");
            exit(EXIT_FAILURE);
         }

        close(sockfd);
        return 0;
}

void check_arg(int argc, char* arg){

	if ( strcmp(arg,"add") && strcmp(arg,"remove") && strcmp(arg,"help") && strcmp(arg,"list") ) 
	{
		printf("imonitor: %s is not a valid argument\n", arg);
		synopsis();
		exit(EXIT_FAILURE);
	}
	if ( !strcmp(arg,"help") ) {
		help();
		exit(EXIT_FAILURE);
	}
	else if ( !strcmp(arg,"add") || !strcmp(arg,"remove")) {
		if (argc<3){
			printf("imonitor: watch descriptor or path required\n");
			exit(EXIT_FAILURE);
		}
	}

}

void synopsis(){
	printf("$imonitor [add|remove|list|help]\n");
}

void help(){
	printf("\nimonitor: client for imonitord to manage multiple inotify watches\n\n\
commands:\n\
$ imonitor add [PATH]       # request a new watch on specified path\n\
$ imonitor remove [WD|PATH] # request to remove watch on specified path (if found)\n\
$ imonitor list             # request a list of paths for running watches\n\
$ imonitor help             # prints this help\n\
");
}
