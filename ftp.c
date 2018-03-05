#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "utils.h"


transport FTPExecute(transport input, char *ftp_dir)
{
	transport response = {input.size, input.message};
	return response;
}