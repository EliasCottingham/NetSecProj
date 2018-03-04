// FTP Client
// Group 1

// #include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int cmd_helper(char cmd[]);

int main(int argc, char *argv[]){
  /*Client takes port number and ip as parameters.*/
  struct sockaddr_in server;
  int sock, connection;
  char cmd[100];

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
  //send(sock, 
  printf("Ready to accept commands: \n\t'put <filename> - will upload a file\n\t'get <filename>'' - will get a file\n\t'ls' - lists the files on the FTP server\n\t'exit' - quit the FTP client");
  while(1){
    printf("\n->");
    fgets(cmd, 100, stdin);
    switch(cmd_helper(cmd)){
      case(0):
        printf("Put cmd.");
        break;
      case(1):
        printf("Get cmd.");
        break;
      case(2):
        printf("LS cmd.");
        break;
      case(3):
        printf("Exit cmd.");
        exit(1);
      case(-1):
        printf("Error.");
        break;
    }
  }
}

int cmd_helper(char cmd[]){
  if (strncmp(cmd, "put ", 4)==0){
    return 0;
  }
  else if (strncmp(cmd, "get ", 4)==0){
    return 1;
  }
  else if (strncmp(cmd, "ls ", 3)==0){
    return 2;
  }
  else if (strncmp(cmd, "exit ", 5)==0){
    return 3;
  }
  return -1;
}
