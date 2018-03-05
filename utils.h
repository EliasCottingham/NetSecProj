#ifndef TRANSPORT_FOR_IDS
#define TRANSPORT_FOR_IDS

#define CHUNK 1500

typedef struct 
{
	size_t size;
	char *message;
} transport;

void ErrorOut(char *msg);

transport FTPExecute(transport input, char *ftp_dir);

void IDSHandler(int client_socket, transport ids_signatures[], char * ftp_dir);

#endif