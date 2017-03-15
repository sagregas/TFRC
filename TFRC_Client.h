/*********************************************************
*
* Module Name: TFRC client header file
*
* File Name:    TFRC_Client.h	
*
* Summary:
*  This file contains variables for client
*
* Suyog Tulapurkar and Sagar Thakur
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
#include <math.h>
#include <stdio.h>


void nofeedbackTimer_Handler();
void clientCNTCCode();
void CatchAlarm();
long nfbTimerUpdate();

char feedbackArrived;
char timerExpired;

int bStop;
int sock;
long *numOfPacketSent;
long *numOfAckRecd;
struct timeval *theTime1;
struct timeval *theTime2;
struct timeval *theTime3;
struct timeval *theTime4;
struct timeval TV1, TV2, TV3, TV4;

long usec1;
long usec2;
long usec3;
long usec4;
long usec1_z;
char currentState;
    
long noFeedBackTimer;

/*Terminal Parameters for TFRC*/
int messageSize;
int connectionID;
int simulatedLossRate;
int maxAllowedThrough;


/*socket parameters*/
struct sockaddr_in echoServAddr; /* Echo server address */
struct sockaddr_in fromAddr;     /* Source address of echo */
unsigned short echoServPort;     /* Echo server port */
unsigned int fromSize;           /* In-out of address size for recvfrom() */

    struct Control_Message controlMessage, controlMessageR;



















