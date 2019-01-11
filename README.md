1. Run tcpServer executable on server. Then all clients who run tcpClient with
a registered identity should be able to connected to the server. Identity 
verification: Client send password and username to server. Server has a hash 
table where username is the key, hash code of the password is the data. If the 
username is found and the hash code of the password matches, the connection can
be built.
2. Server and client side console should print suitable messages to describe 
connections and sent/received packets.
3. Client can send 2 integers to the server, then server sends back the sum.
4. Setup connection ending signals.
5. Deal with overflow, illegal inputs. 
