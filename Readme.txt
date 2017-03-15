TFRC Client-Server application

The code for the server application is in the file TFRC_Server.c and the code for the client application is in the file TFRC_Client.c

The file Client_common.c runs the calculation functions called by TFRC_Client.c.

The file Global.h is a header file which defines certain macros and structures which are required by both server and the client applications.

The file Makefile is used to compile the .c files.

The invocation of the server application is as follows - 

./TFRC_Server <port number>

Where the port number is the destination port number that the server listens on waiting for clients

The invocation of the client application is as follows - 

./TFRC-Client <destinationAddress> <destinationPort> <messageSize>  <connectionID> <simulatedLossRate> <maxAllowedThroughput>

The application can be terminated by issuing a CNTL + C at either the client or the server side.
