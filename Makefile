GCCFLAGS = -ggdb -Wall -std=c11

all: tcpClient serverRegister tcpServer

tcpServer: tcpServer.o hashPassword.o
	gcc $(GCCFLAGS) -o tcpServer tcpServer.o hashPassword.o -lssl -lcrypto

tcpServer.o: tcpServer.c common.h hashPassword.h
	gcc $(GCCFLAGS) -c tcpServer.c

serverRegister: serverRegister.o hashPassword.o
	gcc $(GCCFLAGS) -o serverRegister serverRegister.o hashPassword.o -lssl -lcrypto 

serverRegister.o: serverRegister.c common.h hashPassword.h
	gcc $(GCCFLAGS) -c serverRegister.c

hashPassword.o: hashPassword.c hashPassword.h
	gcc $(GCCFLAGS) -c hashPassword.c

tcpClient: tcpClient.o
	gcc $(GCCFLAGS) -o tcpClient tcpClient.o

tcpClient.o: tcpClient.c common.h
	gcc $(GCCFLAGS) -c tcpClient.c
