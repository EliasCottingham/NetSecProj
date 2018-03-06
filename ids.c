#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>


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

char *ScanData(char *data, int length, transport signatures[])
{
  char *delim ="|";
  printf("Scandata: #%s# which is of length %d\n",data, strlen(data));
  if(strlen(data) ==1){
			printf("Early return\n");
      return "";
  }
	//iterate through signatures checking if data matches signature
	for(;signatures != NULL; signatures){
		printf("Signatures message: %s\n", signatures->message);
		char sig_copy[signatures->size];
		memcpy(sig_copy, signatures->message,signatures->size);
		//get id and data
		char *id = strtok(sig_copy, delim);
		char *info = strtok(NULL,delim);
		printf("ID: #%s# info: #%s#\n", id, info);
		//use memmem incase of null bytes in message
		if(memmem(data, length-1, info, signatures->size-strlen(id)) !=NULL){
			printf("This signature matched: #%s# this piece of data: #%s#\n", info, data);
			return id;
		}
	}
	printf("Normal okay return\n");
  return "";
}

void IDSHandler(int client_socket, transport ids_signatures[], char * ftp_dir, char * ids_logname,char *ip)
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
			char *scan_result =ScanData(buffer, actual_receive_size, ids_signatures);
			printf("scan result: #%s# which is of length %d", scan_result, strlen(scan_result));
			if(strlen(scan_result) ==0)
			{
				printf("Returned from first scandata no issue\n");
				memcpy((message+size_holder), buffer, actual_receive_size);
				size_holder += actual_receive_size;
			} else {
				printf("Scandata bad packet first\n");
				FILE *ids_log = fopen(ids_logname, "a");

				time_t t;
				time(&t);
				struct tm *tm = localtime(&t);
				char time_buffer[80];
				strftime(buffer, 80, "%c", tm);

				//log message format is: <id> <ip> <timestamp>\n
				char message_to_write[strlen(scan_result) + 1 + strlen(ip) + 1 + strlen(time_buffer) +2];
				sprintf("%s %s %d\n", scan_result, ip, time_buffer);
				fwrite(message_to_write, sizeof(char), strlen(message_to_write)+1,ids_log);
				fclose(ids_log);
				//TODO: Do something with bad packets
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
			char *scan_result =ScanData((response.message+size_holder), send_size, ids_signatures);
			printf("scan result: #%s# which is of length %d", scan_result, strlen(scan_result));
			if(strlen(scan_result) ==0)
			{
				printf("Second scandata no issue\n");
				send(client_socket, (response.message+size_holder), send_size, 0);
				size_holder += send_size;
			} else {
				printf("Second scandata bad\n");
				FILE *ids_log = fopen(ids_logname, "a");
				time_t t;
				time(&t);
				struct tm *tm = localtime(&t);
				char time_buffer[80];
				strftime(buffer, 80, "%c", tm);

				//log message format is: <id> <ip> <timestamp>\n
				char message_to_write[strlen(scan_result) + 1 + strlen(ip) + 1 + strlen(time_buffer) +2];
				sprintf("%s %s %d\n", scan_result, ip, time_buffer);
				printf("Got to fwrite\n");
				fwrite(message_to_write, sizeof(char), strlen(message_to_write)+1,ids_log);
				printf("Passed fwrite\n");
				fclose(ids_log);
				printf("Passed fclose");
				//TODO: Do something with bad packets

			}
		}
    printf("Message sent!\n\n");
		free(message);
		free(response.message);
	}
	break_from_receiving:
	printf("GOING TO WAIT FOR NEW CONNECTION\n");
}
