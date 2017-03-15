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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>  

#include "Global.h"
#include "TFRC_Server.h"


char Version[] = "1.1";  

void calcLossRateEvent();
void Prepare_AckMessage(struct Ack_Message *ptr, struct Data_Message *ptr1, long *usec);
void prepare_controlMessage(struct Control_Message *ptr,struct Control_Message *ptr1);
void update_Parameters(struct Data_Message *dataPtr, long *arrivalTime);
  //struct Data_Message DataMessage;

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */

    unsigned short echoServPort;     /* Server port */
    struct Control_Message controlMessage, controlMessageR;
    struct Data_Message DataMessage;
    struct Ack_Message AckMessage;

    theTime1 = &TV1;
    theTime2 = &TV2;
    theTime3 = &TV3;
    theTime4 = &TV4;

    
    signal(SIGINT, serverCNTCCode);
    currentState = SERVER_START;

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    printf("UDPEchoServer(version:%s): Port:%d\n",(char *)Version,echoServPort);    

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
      printf("Failure on socket call , errno:%d\n",errno);
    }

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
          printf("Failure on bind, errno:%d\n",errno);
    }
  

    gettimeofday(theTime1, NULL);
    usec1 = (theTime1->tv_sec) * 1000000 + (theTime1->tv_usec);

    while(1) /* Run forever */
    {
        if (bStop == 1) exit(0);  /* This is to stop sending the data in case of CNTL+C */
        int cliAddrLen = sizeof(echoClntAddr);
        switch(currentState)
        {
            case SERVER_START:
                /* Block until receive message from a client */
            if ((sizeof(controlMessageR) == recvfrom(sock, &controlMessageR, sizeof(controlMessageR), 0,
                (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            {
                printf("Failure on recvfrom, client: %s, errno:%d\n", inet_ntoa(echoClntAddr.sin_addr),errno);
            }
            prepare_controlMessage(&controlMessageR, &controlMessage);
            
            if (sendto(sock, &controlMessage, sizeof(controlMessage), 0,  
             (    struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(controlMessage)) {
                  printf("Failure on sendTo, client: %s, errno:%d\n", inet_ntoa(echoClntAddr.sin_addr),errno);
            }
            else
            {
                currentState = SERVER_DATA;
            }
            
            break;

            case SERVER_DATA:
                    /*put a condition for stop*/
                if ((sizeof(DataMessage) == recvfrom(sock, &DataMessage, sizeof(DataMessage), 0,
                (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
                {
                    if((DataMessage.messageType = CONTROL)&&(DataMessage.messageCode = STOP))
                    {
                        currentState = SERVER_STOP;
                        break;
                    }
                    printf("Failure on recvfrom, client: %s, errno:%d\n", inet_ntoa(echoClntAddr.sin_addr),errno);
                }
                else
                {
                        messageSize = DataMessage.messageLength;
                      
                        gettimeofday(theTime3, NULL);
                        usec3 = (theTime3->tv_sec) * 1000000 + (theTime3->tv_usec);
                        numOfPacketRecd++;
                        serverParameters.X_Recv = (double)(numOfPacketRecd*DataMessage.messageLength*8)/(usec3-usec1);
                       
                        update_Parameters(&DataMessage, &usec3);
                        Prepare_AckMessage(&AckMessage, &DataMessage, &usec3);

                        if (sendto(sock, &AckMessage, sizeof(AckMessage), 0,  
                        (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(AckMessage)) {
                        printf("Failure on sendTo, client: %s, errno:%d\n", inet_ntoa(echoClntAddr.sin_addr),errno);
                        }
                        else
                        {
                            numOfPacketSent++;
                        }
                    
                }
            break;

            case SERVER_STOP:
                   if ((sizeof(controlMessageR) == recvfrom(sock, &controlMessageR, sizeof(controlMessageR), 0,
                    (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
                    {
                    
                    printf("Failure on recvfrom, client: %s, errno:%d\n", inet_ntoa(echoClntAddr.sin_addr),errno);
                }

                           controlMessage.messageLength = 0x0E;
                           controlMessage.messageType = CONTROL;
                           controlMessage.messageCode = controlMessageR.messageCode;
                           controlMessage.messageCxID = controlMessageR.messageCxID;
                           controlMessage.sequenceNumber = controlMessageR.sequenceNumber;
                           controlMessage.messageSize = controlMessageR.messageSize;   

                       if (sendto(sock, &controlMessage, sizeof(controlMessage), 0,  
                       (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(controlMessage)) {
                       printf("Failure on sendTo, client: %s, errno:%d\n", inet_ntoa(echoClntAddr.sin_addr),errno);
                       }   
                     close(sock);
                    serverCNTCCode();
            break;

        }
    }
 }

void serverCNTCCode() {
   gettimeofday(theTime4, NULL);
   usec4 = (theTime4->tv_sec) * 1000000 + (theTime4->tv_usec);
   bStop = 1;
   currentState = SERVER_STOP;
   printf("\nTotal Time: %ld Total Packets Sent: %ld Total Bytes sent: %ld Total Acks Received: %ld Total Avg Throughput: %f Avg LossEventRate: %lf  \n", usec4-usec1, numOfPacketSent, numOfPacketSent*messageSize, numOfPacketRecd, (float)((numOfPacketSent*messageSize*8)/((usec4-usec1)/1000000)), serverParameters.lossEventRate/*AvgLossRate*/);
exit(0);
}

void update_Parameters(struct Data_Message *dataPtr, long *arrivalTime)
{
    long loss_seq = 0;
    long  S_loss = 0;
    char k;
    long T_loss = 0;
    static long loss_seqz = 0;
    static long i = 0;
    static long temp = 0;
    static char flag = RESET;
    static int  j = 0;
    static char firsttime = RESET;

    serverParameters.RTT = dataPtr->Sender_RTT;
    packet_History[i][0] = dataPtr->sequenceNumber;
    packet_History[i][1] = *arrivalTime;
   
    /*determine packet loss*/
    
    
    if(i >= 1)
    {
        if(((packet_History[i-1][0]) + 1) != (packet_History[i][0]))
        {
           loss_seq = i;
           flag = SET;
        }
    }
    if((temp < 3)&&(packet_History[i][0]>packet_History[loss_seq][0])&&(flag == SET))
    {
        temp++;
    }
    
    if(temp >= 3)
    {
        temp = 0;
        flag = RESET;
        S_loss = ((packet_History[loss_seq - 1][0]) + (packet_History[loss_seq][0]))/2;
       
        /*start of a loss rate event*/
        if((packet_History[loss_seq][0]) != (packet_History[loss_seq - 1][0]))
        {
                T_loss = packet_History[loss_seq - 1][1] + (((packet_History[loss_seq][1]) - (packet_History[loss_seq - 1][1]))*(S_loss - packet_History[loss_seq - 1][0])/
                                                   ((packet_History[loss_seq][0]) - (packet_History[loss_seq - 1][0])));
        }
        if((T_loss + serverParameters.RTT) >= (packet_History[loss_seq][1]))
        {
             ;/*same loss rate event*/
        }
        else
        {
             /*New loss rate event*/
            if(firsttime == RESET)
            {
                lossRateEvent[0] = packet_History[loss_seq - 1][0];
                firsttime = SET;
                loss_seqz = loss_seq;
           }
           else
           {
                if(j>7)
                {
                        for(k = 1; k <= 7; k++)
                        {
                            lossRateEvent[k-1] = lossRateEvent[k];
                        }
                       j = 7;
                }
                    lossRateEvent[j] = packet_History[loss_seq - 1][0] - packet_History[loss_seqz][0] + 1;
		   	
                    loss_seqz = loss_seq;
                    j++;
            }               
        }         
    }
    if((i >= 1)&&(packet_History[i-1][0] + 1 == packet_History[i][0]))
    {
         calcLossRateEvent();
    }
    
    i++;
    if(i > 499998)
    {
       i = 3;
    }
}

void calcLossRateEvent()
{
    int w[8] = {0};
    int i = 0;
    int I_tot0 = 0;
    int I_tot1 = 0;
    int W_tot = 0;
    double I_mean;
    int I_tot;

    for(i = 0; i<8;i++)
    {
        if(i < 4)
        {
            w[i] = 1;
        }
        else
        {
            w[i] = 1 - ((i - 3)/5);
        }
    }
    for (i = 0; i<8; i++) 
    {
        I_tot0 = I_tot0 + ((lossRateEvent[i]) *(w[i]));
        W_tot = W_tot + w[i];
	
    }
    for (i = 1; i<= 8 ;i++) 
    {
        I_tot1 = I_tot1 + ((lossRateEvent[i]) * (w[i-1]));
    }
    I_tot = MAX(I_tot0, I_tot1);
    I_mean = I_tot/W_tot;
    serverParameters.lossEventRate = (double)(1/I_mean);
    
}

void Prepare_AckMessage(struct Ack_Message *ptr, struct Data_Message *ptr1, long *usec)
{
    memset(ptr, 0, sizeof(*ptr));
    gettimeofday(theTime4, NULL);
    usec4 = (theTime4->tv_sec) * 1000000 + (theTime4->tv_usec);
    ptr->messageLength = 0x20;
    ptr->messageType = ACK;
    ptr->messageCode = OK;
    ptr->messageCxID = ptr1->messageCxID;
    ptr->acknowlNumber = (ptr1->sequenceNumber) + 1;
    ptr->TimeStamp = usec4;
    ptr->t_delay = usec4 - *usec;
    ptr->lossEventRate = (int)(FLOOR(serverParameters.lossEventRate));
    ptr->receiveRate = (int)(FLOOR(serverParameters.X_Recv));
}

void prepare_controlMessage(struct Control_Message *ptr,struct Control_Message *ptr1)
{
    ptr1->messageLength = 0x0E;
    ptr1->messageType = CONTROL;
    ptr1->messageCode = ptr->messageCode;
    ptr1->messageCxID = ptr->messageCxID;
    ptr1->sequenceNumber = ptr->sequenceNumber;
    ptr1->messageSize = ptr->messageSize;   
}
