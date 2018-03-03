#ifndef TRANSPORT_FOR_IDS
#define TRANSPORT_FOR_IDS

struct transport
{
	int size;
	char *message;
};

void ErrorOut(char *msg);
char *FTPExecute();

void IDSHandler(int client_socket, char *argv[]);

#endif