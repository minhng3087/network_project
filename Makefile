CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 
all: client server

client: client.o file.o drawUtils.o handle.o action.o menu.o
	${CC} client.o file.o drawUtils.o handle.o action.o menu.o -o client -pthread

server: server.o file.o  action.o handle.o
	${CC} server.o file.o  action.o handle.o -o server -pthread

server.o: server.c
	${CC} ${CFLAGS} server.c -pthread

client.o: client.c
	${CC} ${CFLAGS} client.c -pthread

file.o: file.c
	${CC} ${CFLAGS} file.c

action.o: action.c
	${CC} ${CFLAGS} action.c 

menu.o: menu.c
	${CC} ${CFLAGS} menu.c 

drawUtils.o: utils/drawUtils.c
	${CC} ${CFLAGS} utils/drawUtils.c

handle.o: utils/handle.c 
	${CC} ${CFLAGS} utils/handle.c 

clean:
	rm -f *.o *~