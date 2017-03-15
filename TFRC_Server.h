/*********************************************************
*
* Module Name: TFRC server 
*
* File Name:    TFRC_Server.c	
*
* Summary:
*  This file contains the TFRC server code
*
* Suyog Tulapurkar and Sagar Thakur
*
*********************************************************/
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>



void DieWithError(char *errorMessage);  /* External error handling function */
void serverCNTCCode();

long packet_History[500000][2] = {0};
long  lossRateEvent[8] = {0};
long numOfPacketSent;
long numOfPacketRecd;
char currentState;
float totPacketLossRate;

struct timeval *theTime1;
struct timeval *theTime2;
struct timeval *theTime3;
struct timeval *theTime4;
struct timeval TV1, TV2, TV3, TV4;

long usec1;
long usec2;
long usec3;
long usec4;

int messageSize;
int bStop;


