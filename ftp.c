#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "utils.h"


transport FTPExecute(uint32_t size, char *message, char *ftp_dir)
{
	transport response = {size, message};
	return response;
}