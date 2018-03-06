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

int ScanData(char *data, int length, transport signatures[])
{
  //TODO: Need to scan each recieved buffer for signatures
  return 1;
}

void IDSHandler(int client_socket, transport ids_signatures[], char * ftp_dir)
{

	char buffer[CHUNK];
	memset(buffer, 0, sizeof(buffer));

	int32_t net_size;
	int32_t size;
	// char size_buffer[sizeof(size_t)];
	char *size_buffer = (char*)&net_size;
	memset(size_buffer, 0, sizeof(int32_t));

	char *message;

	transport response;
	int actual_receive_size;
	int expected_receive_size;
	int send_size;
	int size_holder;

	while(1)
	{
    //TODO: Seriosuly fucked up.. not sure what to do as of now
		printf("IN IDS RECEIVING LOOP\n");
		if(recv(client_socket, size_buffer, sizeof(int32_t), 0) <= 0){
			printf("READ 0 bytes FROM CLOSED CLIENT SOCKET\n");
			break;
		}

		// size = (uint32_t)*size_buffer;
		size = ntohl(net_size);
		printf("Expected Size: %d\n", size);
		size_holder = 0;
		message = (char *) calloc(size, 1);
		printf("RECEIVE LOOP START:\n");
		while(size_holder < size)
		{
			memset(buffer, 0, sizeof(buffer));
			expected_receive_size = ((size-size_holder) < CHUNK) ? (size-size_holder): CHUNK;
			printf("expected_receive_size: %d size_holder: %d size: %d\n", expected_receive_size, size_holder, size);
			actual_receive_size = recv(client_socket, buffer, expected_receive_size, 0);
			printf("actual_receive_size: %d\n",actual_receive_size);
      		// printf("Recieved: %s\n", buffer);
			if(actual_receive_size <= 0){
				printf("READ 0 bytes FROM CLOSED CLIENT SOCKET\n");
				goto break_from_receiving;
			}
			printf("Receive buffer content: %s\n", buffer);
			if(ScanData(buffer, actual_receive_size, ids_signatures))
			{
				memcpy((message+size_holder), buffer, actual_receive_size);
				size_holder += actual_receive_size;
			}
		}
		printf("\nRECEIVE LOOP END\nSize expected: %d, Size received: %d\n", size, size_holder);

		printf("Message without cmd type: %s\n", (message+1));
		printf("Size: %u\n", size);

		transport input = {size, message};
		response = FTPExecute(input, ftp_dir);

		printf("Message response strlen:%lu\n", strlen((response.message+1)));
		printf("Message response size: %d\n", response.size);
		printf("Message response: %s\n", response.message);
		printf("\n\n\n");

		// send(client_socket, response.message, response.size, 0);

		size_holder = 0;
		printf("SEND LOOP START:\n");
    net_size = htonl(response.size);
		send(client_socket, &net_size, sizeof(int32_t), 0);
		while(size_holder < response.size)
		{
			send_size = ((response.size-size_holder) < CHUNK) ? (response.size-size_holder): CHUNK;
			printf("send_size: %d\n", send_size);
			if(ScanData((response.message+size_holder), send_size, ids_signatures))
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
