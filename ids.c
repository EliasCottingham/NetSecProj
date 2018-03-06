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
  //TODO: this is never used. Whats it for?
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
	int actual_receive_size;
	int expected_receive_size;
	int send_size;
	int size_holder;
  int32_t net_size;
  int32_t size;

  transport response;
  char *message;

	char buffer[CHUNK];
	memset(buffer, 0, sizeof(buffer));

	char *size_buffer = (char*)&net_size;
	memset(size_buffer, 0, sizeof(int32_t));


	while(1)
	{
		printf("IN IDS RECEIVING LOOP\n");
		if(recv(client_socket, size_buffer, sizeof(int32_t), 0) <= 0){
			printf("READ 0 bytes FROM CLOSED CLIENT SOCKET\n");
			break;
		}

		size = ntohl(net_size);
		printf("Expected Size: %d\n", size);
		size_holder = 0;
		message = (char *) calloc(size, sizeof(char));
		printf("RECEIVE LOOP START:\n");
		while(size_holder < size)
		{
			memset(buffer, 0, sizeof(buffer));
			expected_receive_size = ((size-size_holder) < CHUNK) ? (size-size_holder): CHUNK;
			printf("expected_receive_size: %d size_holder: %d size: %d\n", expected_receive_size, size_holder, size);
			actual_receive_size = recv(client_socket, buffer, expected_receive_size, 0);
			printf("actual_receive_size: %d\n",actual_receive_size);

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

		transport input = {size, message};
		response = FTPExecute(input, ftp_dir);

		size_holder = 0;
		printf("SEND LOOP START:\n");
    net_size = htonl(response.size);
		send(client_socket, &net_size, sizeof(int32_t), 0);
		while(size_holder < response.size)
		{
			send_size = ((response.size-size_holder) < CHUNK) ? (response.size-size_holder): CHUNK;
			printf("Current packet size: %d\n", send_size);
			if(ScanData((response.message+size_holder), send_size, ids_signatures))
			{
				send(client_socket, (response.message+size_holder), send_size, 0);
				size_holder += send_size;
			}
		}
    printf("Message sent!\n\n");
		free(message);
	}
	break_from_receiving:
	printf("GOING TO WAIT FOR NEW CONNECTION\n");
}
