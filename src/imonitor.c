#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// imonitor.c | unix socket client for managing imonitor daemon = (daemon.c)
// USAGE: monitor [add|remove|list|help]

void check_arg(int argc, char* arg);
void synopsis();
void help();

int main(int argc, char *argv[])
{

	if (argc == 1){
      	   printf("imonitor: no arguments\n");
	   synopsis();
	   exit(1);
	}

	check_arg(argc, argv[1]);

        int sockfd, t, len;
        struct sockaddr_un remote;
	char *str[100];
	char *request_str;

        if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        {
                perror("Socket");
                exit(1);
        }

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, "/var/run/imonitor.socket");
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);

        if(connect(sockfd, (struct sockaddr *)&remote, len) == -1)
        {
		// perror("Connect");
		printf("imonitor: error connecting to daemon at unix:///var/run/imonitor.socket\n");
		printf("imonitor: run 'systemctl start imonitord' then retry\n");
                exit(1);
        }

        printf("Connected\n");

	// REQUEST FORMAT = "ACTION:PATH"
	// char *request_str = strcat(argv[1], argv[2]);
	
	char *_request_str = strcat(argv[1], "_" );
	request_str = strcat(_request_str, argv[2]);

        if(send(sockfd, request_str, strlen(request_str), 0) == -1)
        {
            perror("Send");
            exit(1);
        }

        if((t = recv(sockfd, request_str, 100, 0)) > 0)
        {
            str[t] = '\0';
            printf("echo> %s", request_str);
        }
         else
         {
            if(t < 0) perror("recv");
            else printf("Server closed connection");
            exit(1);
         }

        close(sockfd);
        return 0;
}

void check_arg(int argc, char* arg){

	if ( strcmp(arg,"add") && strcmp(arg,"remove") && strcmp(arg,"help") && strcmp(arg,"list") ) 
	{
		printf("imonitor: %s is not a valid argument\n", arg);
		synopsis();
		exit(1);
	}
	if ( !strcmp(arg,"help") ) {
		help();
		exit(1);
	}
	else if ( !strcmp(arg,"add") || !strcmp(arg,"remove")) {
		if (argc<3){
			printf("imonitor: watch descriptor or path required\n");
			exit(1);
		}
	}

}

void synopsis(){
	printf("$imonitor [add|remove|list|help]\n");
}

void help(){
	printf("\nimonitor: simple client for imonitord for managing inotify watches\n\n\
commands:\n\
$ imonitor add [PATH] # request a new watch on specified path\n\
$ imonitor remove [PATH] # request to remove watch on specified path (if found)\n\
$ imonitor list       # request a list of paths for running watches\n\
$ imonitor help          # prints this help\n\
");

}
