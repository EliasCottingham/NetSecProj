#ifndef TRANSPORT_FOR_IDS
#define TRANSPORT_FOR_IDS

typedef struct 
{
	int size;
	char *message;
} transport;

void ErrorOut(char *msg);

transport FTPExecute(uint32_t size, char *message, char *ftp_dir);

void IDSHandler(int client_socket, FILE *ids_file, char *ftp_dir);

#endif