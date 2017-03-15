all: TFRC_Client TFRC_Server

TFRC_Server: TFRC_Server.o 
	gcc -g -Wall TFRC_Server.o -o TFRC_Server -lm

TFRC_Client:TFRC_Client.o Client_common.o
	gcc -g -Wall -o TFRC_Client TFRC_Client.o Client_common.o -lm

TFRC_client.o: TFRC_client.c
	gcc -g -Wall -o TFRC_client.o -c TFRC_client.c -lm

Client_common.o: Client_common.c
	gcc -g -Wall -c Client_common.c -lm

TFRC_Server.o: TFRC_Server.c
	gcc -g -Wall -c TFRC_Server.c -lm

clean:
	rm -f *.o *~ TFRC_Client TFRC_Server core

backup: clean
	rm -f *.tar.gz
	tar -cvf TFRC.tar *
	gzip TFRC.tar

