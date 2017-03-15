/*********************************************************
*
* Module Name: TFRC client/server header file
*
* File Name:    Global.h	
*
* Summary:
*  This file contains common stuff for the client and server
*
* Revisions:
*
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>     /* for memset() */
#include <netinet/in.h> /* for in_addr */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <unistd.h>     /* for close() */


/*common Macros*/
#ifndef MIN
#define MIN(x,y) (((x) < (y))?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

#define FLOOR(x) x*1000

#define DEFLOOR(x) x/1000

#define SERVER_START 0x01
#define SERVER_DATA 0x02
#define SERVER_STOP 0x03


#define CONTROL 0x01
#define DATA    0x02
#define ACK     0x03
#define START   0x01
#define STOP    0x02
#define OK      0x03
#define SET 0x01
#define RESET 0x00
#define MaxMsgSize 0x05A2
#define SUCCESS 0x01
#define FAILURE -1;


#define filterConstant_Q 0.9

#ifndef LINUX
#define INADDR_NONE  0xffffffff
#endif

/*structure of control message sender -> receiver*/
struct Control_Message{
    short messageLength;
    char  messageType;
    char  messageCode;
    int   messageCxID;
    int   sequenceNumber;
    short messageSize;
};
/*structure of data message sender -> receiver*/
struct Data_Message
{
    short  messageLength;
    char   messageType;
    char   messageCode;
    int    messageCxID;
    int    sequenceNumber;
    long   TimeStamp;
    int    Sender_RTT;
    char*  messageData;
};
/*structure of Acknowledgement message receiver -> sender*/
struct Ack_Message
{
    short  messageLength;
    char   messageType;
    char   messageCode;
    int    messageCxID;
    int    acknowlNumber;
    long   TimeStamp;
    int    t_delay;
    int    lossEventRate;
    int    receiveRate;
};
void DieWithError(char *errorMessage);  /* External error handling function */

struct Client_Parameters
{
    //double lossEventRate;
    long RTT;
    long R_sample;
    long t_RTO;
    
    long t_now;
    long t_recvdata;
    long tld; 
    
    double p;
    int b; /*number of acknowledgement recieved by client per packet sent*/    

    double X;
    double X_calc;
    double x_Recd;

};

struct Client_Parameters *clientParameters;

/*Global structure used by the server*/
struct Server_Parameters
{
   long RTT;
   double lossEventRate;
   double X_Recv;

};


struct Server_Parameters serverParameters;



