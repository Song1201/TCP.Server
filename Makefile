GCCFLAGS = -ggdb -Wall -std=c11

tcpServer: tcpServer.o 
	gcc $(GCCFLAGS) -o tcpServer tcpServer.o

tcpServer.o: tcpServer.c
	gcc $(GCCFLAGS) -c tcpServer.c

tcpClient: tcpClient.o
	gcc $(GCCFLAGS) -o tcpClient tcpClient.o

tcpClient.o: tcpClient.c
	gcc $(GCCFLAGS) -c tcpClient.c