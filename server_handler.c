#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "utils.h"


int main(int argc, char *argv[])
{
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        ErrorOut("signal() failed");

    if(argc != 3)
    	ErrorOut("Incorrect number of args.\n");

    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;
    unsigned short server_port;
    struct sockaddr_in client_addr;
    unsigned int client_len;

    char *ftp_dir;

    if((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    	ErrorOut("Socket creation failed");

    server_port = atoi(argv[1]);

    ftp_dir = argv[2];

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    	ErrorOut("bind failed");

    if(listen(server_socket, 5) < 0)
    	ErrorOut("Listen failed");

    while(1)
    {
    	client_len = sizeof(client_addr);

    	if((client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len)) < 0)
    		ErrorOut("accept failed");

    	IDSHandler(client_socket, ftp_dir);
    }
}