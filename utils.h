#ifndef TRANSPORT_FOR_IDS
#define TRANSPORT_FOR_IDS

typedef struct 
{
	int size;
	char *message;
} transport;

void ErrorOut(char *msg);

transport FTPExecute(transport input, char *ftp_dir);

void IDSHandler(int client_socket, char *ids_signatures[], char * ftp_dir);

#endif