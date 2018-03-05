#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void ErrorOut(char *msg)
{
	printf(msg);
	printf("\n");
	exit(1);
}