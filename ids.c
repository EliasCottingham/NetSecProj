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

	char buffer[CHUNK];
	memset(buffer, 0, sizeof(buffer));
	char size_buffer[sizeof(size_t)];
	memset(size_buffer, 0, sizeof(size_buffer));
	uint32_t size;

	char *message;

	transport response;
	int read_size;
	int recv_size;
	int send_size;
	int size_holder;

	while(1)
	{
		printf("IN IDS RECEIVING LOOP\n");
		recv(client_socket, size_buffer, sizeof(size_buffer), 0);
		size = (int) *size_buffer;
		printf("Expected Size: %d\n", size);
		size_holder = 0;
		message = (char *) calloc(size, 1);
		printf("RECEIVE LOOP START:\n");
		while(size_holder < size)
		{
			memset(buffer, 0, sizeof(buffer));
			recv_size = ((size-size_holder) < CHUNK) ? (size-size_holder): CHUNK;
			printf("recv_size: %d size_holder: %d size: %d\n", recv_size, size_holder, size);
			read_size = recv(client_socket, buffer, recv_size, 0);
			printf("read_size: %d\n",read_size);
      printf("Recieved: %s\n", buffer);
			if(read_size <= 0){
				printf("READ 0 bytes FROM CLOSED CLIENT SOCKET\n");
				goto break_from_receiving;
			}
			printf("Receive buffer content: %s\n", buffer);
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

		printf("Message response len:%lu\n", strlen(response.message));
		printf("Message response size: %zu\n", response.size);
		printf("Message response: %s\n", response.message);
		printf("\n\n\n");

		// send(client_socket, response.message, response.size, 0);

		size_holder = 0;
		printf("SEND LOOP START:\n");
		send(client_socket, &response.size, sizeof(response.size), 0);
		while(size_holder < response.size)
		{
			send_size = ((response.size-size_holder) < CHUNK) ? (response.size-size_holder): CHUNK;
			printf("send_size: %d\n", send_size);
			if(ScanData(response.message, send_size, ids_signatures))
			{
				// printf()
				send(client_socket, (response.message+size_holder), send_size, 0);
				size_holder += send_size;
			}
		}
		free(message);
	}
	break_from_receiving:
	printf("GOING TO WAIT FOR NEW CONNECTION\n");
}
