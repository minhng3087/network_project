CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 
all: client server

client: client.o file.o action.o menu.o
	${CC} client.o file.o action.o menu.o -o client

server: server.o file.o action.o
	${CC} server.o file.o action.o -o server -pthread

server.o: server.c
	${CC} ${CFLAGS} server.c -pthread

client.o: client.c
	${CC} ${CFLAGS} client.c

file.o: file.c
	${CC} ${CFLAGS} file.c

action.o: action.c
	${CC} ${CFLAGS} action.c 

menu.o: menu.c
	${CC} ${CFLAGS} menu.c 

clean:
	rm -f *.o *~