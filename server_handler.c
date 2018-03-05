#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"


int main(int argc, char *argv[])
{
		if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) ErrorOut("signal() failed");
	  if(argc != 4) ErrorOut("Wrong number of parameters.\nYou must provide a port, the FTP directory, and then the IDS signature file.\n");

    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;
    unsigned short server_port;
    struct sockaddr_in client_addr;
    unsigned int client_len;
		int len;

    char *ftp_dir;
    char *ids_filename;
    FILE *ids_file;

    if((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) ErrorOut("Socket creation failed");

    // MAKE SURE TO ADD INPUT HANDLING HERE
    server_port = atoi(argv[1]);

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(server_port);
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		if(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
			ErrorOut("bind failed");
		}
		if(listen(server_socket, 5) < 0){
			ErrorOut("Listen failed");
		}

		struct stat sb;
		if(!(stat(argv[2], &sb) == 0 && S_ISDIR(sb.st_mode))){
			printf("Error: Directory provided [%s] does not exist.\n", argv[2]);
			exit(1);
		}

		len = strlen(argv[2]);
		if (argv[2][len-1] != '/'){
			ftp_dir = malloc(len+1);
			strncpy(ftp_dir, argv[2], len);
			ftp_dir[len] = '/';
		} else {
			ftp_dir = argv[2];
		}

		ids_filename = argv[3];
		if (strchr(ids_filename, '/')){
			free(ftp_dir);
			ErrorOut("Error: File name cannot be a path or contian any '/'s. ");
		}
		if(!(stat(ids_filename, &sb) == 0 && !S_ISDIR(sb.st_mode))){
			printf("Error: Directory provided [%s] does not exist.\n", ids_filename);
			free(ftp_dir);
			exit(1);
		}
    ids_file = fopen(ids_filename, "rb");

    char len_buffer[2];


	transport signatures[51];
	memset(signatures, 51*sizeof(transport), 0);
	int i;
	char temp;
	for(i = 0; i < 50; i++){
		// These lines initialize everything to zero
		memset(len_buffer, 2, 0);
		memset(&temp, 1, 0);
		// This read is supposed to get the size of a line. If it gets nothing eof has been reached.
		if(fread(len_buffer, 1, 2, ids_file) == 0)
		{
			printf("Finished reading signatures.\n");
	    	break;
		}

		signatures[i].size = atoi(len_buffer); //This is just the integer length of the
		fread(&temp, 1, 1, ids_file);
		if(temp != '|') ErrorOut("Error reading IDS file on expected pipe ('|') separator. Format per line should be xx|<pattern>\n where xx is two bytes for an integer representing length and <pattern> is the pattern");
		
		signatures[i].message = (char *) calloc(signatures[i].size, 1);
		fread(signatures[i].message, 1, signatures[i].size, ids_file);
		printf("signatures[%d]: %s\n", i, signatures[i].message);
		
		fread(&temp, 1, 1, ids_file);

		if(temp != '\n' && temp != EOF) ErrorOut("Error reading IDS file on expected newline ('\\n'). Format per line should be xx|<pattern>\n where xx is two bytes for an integer representing length and <pattern> is the pattern");
	}

    // ENDING HERE

    while(1)
    {
    	client_len = sizeof(client_addr);
    	if((client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len)) < 0){
				free(ftp_dir);
				ErrorOut("accept failed");
			}
    	IDSHandler(client_socket, signatures, ftp_dir);
    }
}
