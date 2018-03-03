#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "utils.h"



void SendComplete(int socket, const void *msg, int len, int flags)
{
	int sent = 0;
	int sum_sent = 0;
	char *message = (char *) msg;
	while((sent = send(socket, message, len, flags)) < len)
	{
		if(sent == -1)
			ErrorOut("Error on communication with ftp server");

		sum_sent += sent;
		message = ((char *) msg) + sum_sent;
		len-=sent;
	}
	if(sent == -1)
		ErrorOut("Error on communication with ftp server");
}


void IDSHandler(int client_socket, char * ftp_dir)
{
	char buffer[2048];
	memset(buffer, 0, sizeof(buffer));
	char size_buffer[4];
	memset(size_buffer, 0, sizeof(size_buffer));
	uint32_t size;

	char *message;

	transport response;
	while(1)
	{
		recv(client_socket, size_buffer, sizeof(size_buffer), 0);
		size = ntohl((uint32_t) size_buffer);
		// size = atoi(size_buffer);
		message = (char *) calloc(size, 1);
		//CHECK WHAT WAS RECEIVED IN message

		response = FTPExecute(size, message, ftp_dir);
		//CHECK WHAT WAS RECEIVED IN response

		send(client_socket, response.message, response.size, 0);
	}
}