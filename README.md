# TCP Server
A TCP server to allow clients connect to server and let verified user use server's computational resources.

## Compling
In TCP.Server directory, use command
```
make
```
to create all executables. You might need to install libssl-dev before make.

## Deployment
Executable "tcpServer" and configuration file "server.conf" need to be deployed on server machine. "tcpClient" need to be 
deployed on client machine. 

## Usage
Run tcpServer on server machine, then clients can connect to server and login to server on client machine by running tcpClient. Clients need to know the server IPv4 address and listening port number to connect to server. 

## Configuration
1. To change the port server listening on, change the number in the first line in "server.conf".
2. To add new user to the server, run "serverRegister" and follow instructions.
3. To delete a user, open "server.conf" and delete the user's corresponding line.

Restart tcpServer to apply changes.

## Notes
1. In "server.conf", there can only be a number in the first line which represent the port number to be listening on. Each user's 
information is recorded as a line in "server.conf". In each line, the first string is username, the second string is the SHA256 
digest of user's password.
2. The max number of connecting clients is 32, the max number of registered user is 10000 and the max number of connect request in queue is 5. The number can be changed by change the #define statement in tcpServer.c
