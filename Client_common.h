/*********************************************************
* Module Name: Client common
*
* File Name:    Client_common.c
*
* Summary:
*  This file contains the basic functions used by TFRC client.
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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 


void update_Parameters(struct Ack_Message *ptr);

void update_Sendingrate(char nofbtimerElapsed);


