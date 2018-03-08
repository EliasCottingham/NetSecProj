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


char *ScanData(char *data, int length, transport signatures[])
{
  char *delim ="|";
  if(strlen(data) ==1){
			printf("Early return\n");
      return "";
  }
	/*
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
	*/
	char *id;
	for(;signatures->size != NULL; signatures++){
		id = malloc(signatures->size+1);
		memset(id,'\0',signatures->size);
		memcpy(id, signatures->message, signatures->size);
		signatures++;
		char pattern[signatures->size+1];
		memset(pattern,'\0',sizeof(pattern));
		memcpy(pattern, signatures->message,signatures->size);
		if(memmem(data, length-1, pattern,signatures->size)){
			printf("This signature with id %s matched: %s this piece of data %s\n", id, pattern, data);
			return id;
		}
		free(id);
	}
	// if(id != NULL)
	// 	free(id);
  return "";
}

void WriteToLog(char *ids_logname, char *id, char*ip)
{
	FILE *ids_log = fopen(ids_logname, "a");
	time_t t;
	time(&t);
	struct tm *tm = localtime(&t);
	char time_buffer[80];
	strftime(time_buffer, 80, "%c", tm);

	//log message format is: <id> <ip> <timestamp>\n
	char message_to_write[strlen(id) + 1 + strlen(ip) + 1 + sizeof(time_buffer) +2];
	sprintf(message_to_write,"%s %s %s\n", id, ip, time_buffer);
	fwrite(message_to_write, sizeof(char), strlen(message_to_write)+1,ids_log);
	fclose(ids_log);
}

void IDSHandler(int client_socket, transport ids_signatures[], char * ftp_dir, char * ids_logname,char *ip)
{
	int actual_receive_size;
	int expected_receive_size;
	int send_size;
	int size_holder;
  int32_t net_size;
  int32_t size;

  char *send_buffer;

  transport response;
  char *message;

	char buffer[CHUNK];
	memset(buffer, 0, sizeof(buffer));

	char *size_buffer = (char*)&net_size;
	memset(size_buffer, 0, sizeof(int32_t));


	int test_holder;
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
			if(strlen(scan_result) ==0)
			{
				memcpy((message+size_holder), buffer, actual_receive_size);
				size_holder += actual_receive_size;
			} else {
				WriteToLog(ids_logname, scan_result, ip);
				free(scan_result);
				//TODO: Do something with bad packets
				size -= actual_receive_size;
			}

		}
		printf("\nRECEIVE LOOP END\nSize expected: %d, Size received: %d\n", size, size_holder);

		transport input = {size, message};
		response = FTPExecute(input, ftp_dir);
		
		send_buffer = (char *) calloc(response.size, sizeof(char));

		int original_pos;
		int send_buffer_pos = 0;
		// int final_length;
		for(original_pos = 0; original_pos < response.size; original_pos+=CHUNK)
		{
			send_size = ((response.size-original_pos) < CHUNK) ? (response.size-original_pos): CHUNK;
			char *scan_result = ScanData((response.message+size_holder), send_size, ids_signatures);
			if(strlen(scan_result) == 0)
			{
				memcpy((send_buffer + send_buffer_pos), (response.message+original_pos), send_size);
				send_buffer_pos += send_size;
			} else {
				WriteToLog(ids_logname, scan_result, ip);
			}
		}

		net_size = htonl(send_buffer_pos);
		send(client_socket, &net_size, sizeof(int32_t), 0);
		send(client_socket, send_buffer, send_buffer_pos, 0);



		// size_holder = 0;
		// printf("SEND LOOP START:\n");
  //   	net_size = htonl(response.size);
		// send(client_socket, &net_size, sizeof(int32_t), 0);
		// while(size_holder < response.size)
		// {

		// 	printf("response.size: %d size_holder: %d\n", response.size, size_holder);
		// 	send_size = ((response.size-size_holder) < CHUNK) ? (response.size-size_holder): CHUNK;
		// 	printf("Current packet size: %d\n", send_size);
		// 	char *scan_result =ScanData((response.message+size_holder), send_size, ids_signatures);
		// 	printf("Scan result: [%s] %d\n", scan_result, response.size);

		// 	if(strlen(scan_result) == 0)
		// 	{
		// 		send(client_socket, (response.message+size_holder), send_size, 0);
		// 		size_holder += send_size;
		// 	} else {
		// 		//TODO: Do something with bad packets
		// 		WriteToLog(ids_logname, scan_result, ip);
		// 		size_holder += send_size;
		// 	}
		// }
    printf("Message sent!\n\n");
		free(message);
		// }
	}
	break_from_receiving:
	printf("GOING TO WAIT FOR NEW CONNECTION\n");
}
