GCCFLAGS = -ggdb -Wall -std=c11

serverRegister: serverRegister.o
	gcc $(GCCFLAGS) -o serverRegister serverRegister.o -lssl -lcrypto 

serverRegister.o: serverRegister.c common.h
	gcc $(GCCFLAGS) -c serverRegister.c

tcpServer: tcpServer.o 
	gcc $(GCCFLAGS) -o tcpServer tcpServer.o

tcpServer.o: tcpServer.c common.h
	gcc $(GCCFLAGS) -c tcpServer.c

tcpClient: tcpClient.o
	gcc $(GCCFLAGS) -o tcpClient tcpClient.o

tcpClient.o: tcpClient.c common.h
	gcc $(GCCFLAGS) -c tcpClient.c
