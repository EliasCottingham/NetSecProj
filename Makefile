CC  = gcc
CXX = g++

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

.PHONY: default
default: server_handler


server_handler: ids.o ftp.o ErrorOut.o

server_handler.o: server_handler.c

ids.o: ids.c

ftp.o: ftp.c

ErrorOut.o: ErrorOut.c


.PHONY: clean
clean:
	rm -f *.o *~ a.out core server_handler

.PHONY: all
all:
	clean default