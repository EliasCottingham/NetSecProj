// FTP Client
// Group 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define CHUNK 1500

int cmd_helper(char* cmd);
int get_fname(char* cmd, char** fname);
int get_path(char* path, char* fname, char** totalpath);
void recv_response(int* sock, char** response_buffer, int* rec_len);

int main(int argc, char *argv[]){
  /*Client takes port number and ip as parameters.*/
  struct sockaddr_in server;
  int sock, connection, len;
  char cmd[100], buff[CHUNK];
  char* path;
  char* fname;
  char* totalpath;

  if(argc != 4){
    printf("Wrong number of parameters.\nThe FTP client takes a port, ip, and the directory from which it reads and writes.\n");
    exit(1);
  }

  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    printf("Socket creation failed.");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(argv[1]));
  server.sin_addr.s_addr = inet_addr(argv[2]);
  if((connection = connect(sock,(struct sockaddr*)&server, sizeof(server))) == -1){
    printf("Connection failed.\n");
    exit(1);
  }

  struct stat sb;
  if(!(stat(argv[3], &sb) == 0 && S_ISDIR(sb.st_mode))){
    printf("Error: Directory provided [%s] does not exist.\n", argv[3]);
    exit(1);
  }

  len = strlen(argv[3]);
  if (argv[3][len-1] != '/'){
    path = malloc(len+1);
    strncpy(path, argv[3], len);
    path[len] = '/';
  } else {
    path = argv[3];
  }

  printf("Ready to accept commands: \n\t'put <filename> - will upload a file\n\t'get <filename>'' - will get a file\n\t'ls' - lists the files on the FTP server\n\t'exit' - quit the FTP client");
  char* command;
  char* response_buffer;

  len=0;
  int rec_len;
  char cmd_type;
  while(1){
    printf("\n->");
    fflush(stdin);
    fgets(cmd, 100, stdin);
    switch(cmd_type = cmd_helper(cmd)){
      case(0):
        printf("Put cmd.\n");
        if (get_fname(cmd, &fname)==-1){
          free(fname);
          break;
        } else if (get_path(path, fname, &totalpath)==-1){
          free(totalpath);
          break;
        }
        stat(totalpath, &sb);
        // size_t size = htonl(sb.st_size);
        // Size here is the overall size of the message sent to the ids.  Of format <char type of command><string filename><EOF delimited file>
        size_t size = 1+(strlen(fname)+1)+sb.st_size;
        send(sock, &size, sizeof(size), 0);
        send(sock, &cmd_type, sizeof(cmd_type), 0);
        send(sock, fname, strlen(fname)+1, 0);

        FILE *fp;
        fp = fopen(totalpath, "r");
        size_t nread;
        while((nread = fread(buff, 1, CHUNK, fp)) > 0){
          send(sock, buff, nread, 0);
        }
        fclose(fp);
        free(fname);
        free(totalpath);

        recv_response(&sock, &response_buffer, &rec_len);
        printf("Response in case: %s\n", (response_buffer+1));
        free(response_buffer);
        break;

      case(1):
        printf("Get cmd.\n");
        if (get_fname(cmd, &fname)==-1){
          free(fname);
          break;
        }
        len = strlen(fname)+1;
        send(sock, &len, sizeof(len), 0);
        send(sock, &cmd_type, sizeof(cmd_type), 0);
        send(sock, fname, len, 0);
        free(fname);

        recv_response(&sock, &response_buffer, &rec_len);
        printf("%s\n", (response_buffer+1));
        free(response_buffer);
        break;

      case(2):
        printf("LS cmd.\n");
        command = "ls";
        len = strlen(command)+1;
        send(sock, &len, sizeof(len), 0);
        send(sock, &cmd_type, sizeof(cmd_type), 0);

        recv_response(&sock, &response_buffer, &rec_len);
        printf("%s\n", (response_buffer+1));
        free(response_buffer);
        break;

      case(3):
        printf("Exit cmd.\n");
        free(path);
        exit(1);
      case(-1):
        printf("Error: Command not recognived.\n");
        printf("Accepted commands are: \n\t'put <filename> - will upload a file\n\t'get <filename>'' - will get a file\n\t'ls' - lists the files on the FTP server\n\t'exit' - quit the FTP client");
        break;
    }
  }
}


void recv_response(int* sock, char** response_buffer, int* rec_len){
  /* Helper: Handles the response from server. */
  int size;
  int size_holder = 0;
  int exp_size;
  char len_buffer[sizeof(size_t)];
  char buffer[CHUNK];

  printf("WAITING FOR RESPONSE:\n");
  recv(*sock, len_buffer, sizeof(len_buffer), 0);
  *rec_len = (int)*len_buffer;
  printf("Expect Len: %d\n", *rec_len);
  *response_buffer = (char *) calloc(*rec_len, 1);
  while(size_holder < *rec_len)
  {
    memset(buffer, 0, sizeof(buffer));
    exp_size = ((size-size_holder) < CHUNK) ? (size-size_holder): CHUNK;
    size = recv(*sock, buffer, exp_size, 0);
    printf("Buff: %s\n",buffer+1);
    if (size <= 0){
      printf("READ 0 bytes FROM CLOSED CLIENT SOCKET\n");
      break;
    }
    memcpy((*response_buffer+size_holder), buffer, size);
    size_holder += size;
  }
  printf("Received: %s\n", *response_buffer+1);
  printf("Rec_len: %d\n", *rec_len);
}

int get_fname(char* cmd, char** fname){
  /* Helper: Gets the file name and makes sure it is not a directory */
  int len = strlen(cmd);
  *fname = calloc(len-5, sizeof(char));
  strncpy(*fname, cmd+4, len-5);
  if (strchr(*fname, '/')){
    printf("Error: File name cannot be a path or contian any '/'s. ");
    return -1;
  }
  return 1;
}

int get_path(char* path, char* fname, char** totalpath){
  /* Helper: Gets the total path of the file and verifies that its a.) Not a Directory,
  b.) it exists. */
  int len = strlen(path)+strlen(fname);
  *totalpath = malloc(len);
  strncpy(*totalpath, path, strlen(path));
  strcat(*totalpath, fname);
  struct stat sb;
  if (!(stat(*totalpath, &sb) == 0 && !S_ISDIR(sb.st_mode))){
    printf("Error: File provided [%s] is not located with in the current working directory [%s].\n", fname, path);
    return -1;
  }
  return 1;
}

int cmd_helper(char* cmd){
  /* Helper: Gets command type, and returns coresponding int for switch
   cases. */
  if (strncmp(cmd, "put ", 4)==0){
    return 0;
  }
  else if (strncmp(cmd, "get ", 4)==0){
    return 1;
  }
  else if (strncmp(cmd, "ls", 2)==0){
    return 2;
  }
  else if (strncmp(cmd, "exit", 4)==0){
    return 3;
  }
  return -1;
}
