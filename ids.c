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
			ErrorOut("Error on communication with ftp server 1");

		sum_sent += sent;
		message = ((char *) msg) + sum_sent;
		len-=sent;
	}
	if(sent == -1)
		ErrorOut("Error on communication with ftp server 1");
}

int ScanData(char *data, int length, char *signatures[])
{
  //TODO: Need to scan each recieved buffer for signatures
  return 1;
}

void IDSHandler(int client_socket, char *ids_signatures[], char * ftp_dir)
{

	char buffer[1400];
	memset(buffer, 0, sizeof(buffer));
	char size_buffer[4];
	memset(size_buffer, 0, sizeof(size_buffer));
	uint32_t size;

	char *message;

	transport response;
	int read_size;
	int size_holder;

	while(1)
	{
		printf("IN IDS RECEIVING LOOP\n");
		recv(client_socket, size_buffer, sizeof(size_buffer), 0);
		size = (int) *size_buffer;
		printf("SIZE: %d\n", size);
		size_holder = 0;
		message = (char *) calloc(size, 1);
		printf("RECEIVE LOOP START:\n");
		while(size_holder < size)
		{
			memset(buffer, 0, sizeof(buffer));
			read_size = recv(client_socket, buffer, sizeof(buffer), 0);
			printf("Data recieved: %s", buffer);
			if(ScanData(buffer, read_size, ids_signatures))
			{
				memcpy((message+size_holder), buffer, read_size);
				size_holder += read_size;
			}
		}
		printf("\nRECEIVE LOOP END\nSize expected: %d, Size received: %d\n", size, size_holder);
		printf("Message: %s\n", message);
		printf("Size: %u\n", size);
		transport input = {size, message};
		response = FTPExecute(input, ftp_dir);
		//CHECK WHAT WAS PLACED IN response
		printf("Response message strlen: %lu\n", strlen(response.message));
		printf("Response message: %s\n", response.message);
		send(client_socket, response.message, response.size, 0);
		free(message);
	}
}
