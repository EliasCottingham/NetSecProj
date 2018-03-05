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

// int ScanData(char *data, int length, char *signatures[])
// {

// }

void IDSHandler(int client_socket, FILE *ids_file, char * ftp_dir)
{

	char file_holder[5000];
	memset(&file_holder, 0, (50*100));

	// fread(file_holder, 1, 5000, ids_file);

	// if(feof(ids_file))
	// {
	// }
	char len_buffer[2];
	int len;
	char *lines[50];
	int i;
	char temp;
	for(i = 0; i < 50; i++){
		printf("%d\n", i);
		fread(len_buffer, 1, 2, ids_file);
		len = atoi(len_buffer);
		fread(&temp, 1, 1, ids_file);
		if(temp != '|') ErrorOut("Error reading IDS file. Format per line should be xx|<pattern>\n where xx is two bytes for an integer representing length and <pattern> is the pattern");
		lines[i] = (char *) calloc(len, 1);
		fread(lines[i], 1, len, ids_file);
		if(fread(&temp, 1, 1, ids_file) == 0)
		{
			printf("SHOULD BREAK");
			break;
		}
		if(temp != '\n') ErrorOut("Error reading IDS file. Format per line should be xx|<pattern>\n where xx is two bytes for an integer representing length and <pattern> is the pattern");
		printf("%s\n", lines[i]);
	}




	// while(fread(line[i], 100, ids_file))
	// {
	// 	printf("%s\n", line[i]);
	// 	i++;
	// }

	// char *patterns = (char *) calloc(i, 1);
	// int j;
	// for(j=0; j <= i; j++)
	// {

	// 	patterns[j] =
	// }

	char buffer[2048];
	memset(buffer, 0, sizeof(buffer));
	char size_buffer[4];
	memset(size_buffer, 0, sizeof(size_buffer));
	uint32_t size;

	char *message;

	transport response;

	int read_holder;

	while(1)
	{
		read_holder = 0;

		recv(client_socket, size_buffer, sizeof(size_buffer), 0);
		// size = ntohl((uint32_t) size_buffer);
		size = (int) *size_buffer;
		// size = atoi(size_buffer);
		message = (char *) calloc(size, 1);
		recv(client_socket, message, size, 0);
		printf("%.5s\n", message);
		printf("%u\n", size);
		//CHECK WHAT WAS RECEIVED IN size_buffer+message

		response = FTPExecute(size, message, ftp_dir);
		//CHECK WHAT WAS PLACED IN response
		printf("%lu\n", strlen(response.message));
		printf("%s", response.message);
		printf("\n\n\n");
		send(client_socket, response.message, response.size, 0);
		free(message);
	}
}
