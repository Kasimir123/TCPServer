# TCPServer

This is a client and server program implemented in C. The client can send both messages as well as files to the server. The server can handle multiple clients at once and will then log all sent information. This includes the ip address of the client, time, and message/file. If the server is sent a file it stores the file in a folder named after the ip address from where the file was sent and names the file with the format: timestamp filename.

## Client Usage

Edit the connection information at the top of the constants file so that the ip matches the address of the server. Then you can compile with:

```
gcc ./client.c -o client
```

### Send File Over Client

To send a file over the client do 

```
./client -f file/path/name.extension
```

### Send Text Over Client

To send text over the client do 

```
./client -t this is text that will be sent
```

## Server Usage

After specifying whether you want the logs logged to either the terminal or the log file set the OUTPUTTOCOMMANDLINE definition at the top of the server file to either 0 or 1. Then compile with 

```
gcc ./server -o server
```

and simply run with 

```
./server
```
