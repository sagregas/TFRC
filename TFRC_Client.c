/*********************************************************
* Module Name: TFRC client source 
*
* File Name:    TFRC_Client.c
*
* Summary: This file is main file for TFRC_Client
*  
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
#include <sys/mman.h>
#include <errno.h>
#include "Global.h"
#include "TFRC_Client.h"
#include "Client_common.h"

char Version[] = "1.1"; 
void prepControlMessage(struct Control_Message *cntlMsgPtr, int type);
void prepare_DataMessage(struct Data_Message *ptr);
int Packet_Drop(double x);
pid_t child_pid;


int main(int argc, char *argv[])
{
    char *servIP;                    /* IP address of server */
                   
    struct hostent *thehost;	     /* Hostent from gethostbyname() */
    int delay, packetSize;  /* Iteration delay in seconds, packetSize*/
    
    struct sigaction myaction;
    struct Data_Message dataMessage;
    struct Ack_Message ackMessage;
    static char sendPacket = SET;   /*This will send the first packet anyway*/
    static char feedbackArrived = RESET;
    theTime1 = &TV1;
    theTime2 = &TV2;
    theTime4 = &TV4;
    static int *update_param = SET;
    static char timerOK = RESET; 
     /*Initialize parameters*/
     bStop = 0;
     numOfPacketSent = 0;
     numOfAckRecd = 0;
     
     feedbackArrived = RESET;
     timerExpired = RESET;

    if (argc != 7)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Parameters: [<tfrc-client>] [<Dest Server IP>] [<Dest Server Port>] [<messageSize>] [<connectionID>] [<simulatedLossRate>]                     [<maxAllowedThrough>] \n");
        exit(1);
    }

    signal(SIGINT, clientCNTCCode);
    signal(SIGUSR1, nofeedbackTimer_Handler);
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */

    /* get info from parameters , or default to defaults if they're not specified */
    echoServPort      = atoi(argv[2]);
    messageSize       = atoi(argv[3]);
    connectionID      = atoi(argv[4]);
    simulatedLossRate = atoi(argv[5]);
    maxAllowedThrough = atoi(argv[6]);

    if(messageSize >= MaxMsgSize)
    {
        messageSize = MaxMsgSize;  
    }
    if(maxAllowedThrough >= 10000000)
    {
        maxAllowedThrough = 10000000;
    }
   
    currentState = CONTROL;

  
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    
    /* If user gave a dotted decimal address, we need to resolve it  */
    if (echoServAddr.sin_addr.s_addr == -1) {
        thehost = gethostbyname(servIP);
	    echoServAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
    }
    
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */


    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /*Start of control message*/
    prepControlMessage(&controlMessage, START);
    if(0 > sendCtrlMessage(&controlMessage))
    {
        printf("Failed to establish a connection\n");
        exit(1);
    }
    else 
    {
       printf("Connection Established \n");
       currentState = DATA;
    }
         
    /*start sending the data message*/
    if(DATA == currentState)
    {
        clientParameters = mmap(NULL, sizeof(struct Client_Parameters), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
	numOfPacketSent = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
	numOfAckRecd = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
        clientParameters->X = 1;
        clientParameters->tld = 0;

        gettimeofday(theTime2, NULL);
        usec2 = (theTime2->tv_sec) * 1000000 + (theTime2->tv_usec);
         child_pid = fork();

        while(1)
        {
	
           if (bStop == 1) exit(0);  /* This is to stop sending the data in case of CNTL+C */
        
            /*Parent process will send the data packets*/
            if(child_pid > 0)
            {    
		printf("Current Sending Rate = %lf , X_calc = %lf , X_Recv = %lf , loss event rate = %lf\n", clientParameters->X,clientParameters->X_calc,clientParameters->x_Recd , clientParameters->p);
		sleep(2);
   
	
                 if(/*SET == sendPacket*/1)
                 {
                     
                     prepare_DataMessage(&dataMessage); 
		 if(1 == Packet_Drop(simulatedLossRate))	
                    {
                    if(sendto(sock, &dataMessage, sizeof(dataMessage), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(dataMessage))
                     {DieWithError("sendto() sent a different number of bytes than expected");}
                
                     (*numOfPacketSent)++ ;
                     sendPacket = RESET;
                     usec1_z = usec1;
                     timerOK = RESET;
                     gettimeofday(theTime1, NULL);
                     usec1 = (theTime1->tv_sec) * 1000000 + (theTime1->tv_usec);
                     }
                 }
                
                 
                 
            }
            /*Child process is implemented to process the received acknowledgements*/
            else if(child_pid == 0)
            {  
 		
                
                 fromSize = sizeof(fromAddr);
                if((sizeof(ackMessage) == recvfrom(sock, &ackMessage, sizeof(ackMessage), 0,
                (struct sockaddr *) &fromAddr, &fromSize)))
                {
                    feedbackArrived = SET;
                    (*numOfAckRecd)++ ;
                    update_Parameters(&ackMessage);
                    noFeedBackTimer = nfbTimerUpdate();
                }
		
            }
            else
            {
                printf("failed to recognize the process");
                perror("fork");
                exit(0);
            }
        
         if(((clientParameters->RTT) < noFeedBackTimer)&&(SET == feedbackArrived)) // feedback arrived before nofeedbacktimer overflow
         {
            feedbackArrived = RESET;
         }
         /*more reffinement needed*/
         if(((clientParameters->RTT) > noFeedBackTimer)&&(RESET == feedbackArrived)) //no feedback arrived before nofeedbacktimer overflow
         {
             raise(SIGUSR1);
         }
         /*Send the new byte only when currentSending rate allows it*/
        if(((clientParameters->X) != 0) || (timerOK == RESET))
        { 
	     if((usec1-usec1_z) > ((messageSize*8)/(clientParameters->X)))
             {
                 sendPacket = SET;
                 timerOK = SET;
             }
	}
	
       }
    }

}

int sendCtrlMessage(struct Control_Message *ptr)
{
    int firstSeqNo;
    /* Send the control message to the server */

     firstSeqNo = ptr->sequenceNumber; 
    while((ptr->sequenceNumber - firstSeqNo) <= 10)
    {
        //if (bStop == 1) exit(0);

        if (sendto(sock, ptr, sizeof(*ptr), 0, (struct sockaddr *)
               &echoServAddr, sizeof(echoServAddr)) != sizeof(*ptr))
              DieWithError("sendto() sent a different number of bytes than expected");
  
        /* Recv a response */

        fromSize = sizeof(fromAddr);
        alarm(10);            //set the timeout for 10 seconds

        if ((sizeof(controlMessageR) != recvfrom(sock, &controlMessageR, sizeof(controlMessageR), 0,
             (struct sockaddr *) &fromAddr, &fromSize))) {
            (ptr->sequenceNumber)++; 
            if (errno == EINTR) 
            { 
               printf("Received a  Timeout !!!!!\n"); 
               continue; 
            }
        }
        else
        {
            alarm(0);
            return SUCCESS;
        }
        alarm(0);            //clear the timeout 
    }
    if(((ptr->sequenceNumber - firstSeqNo) >= 10)||(controlMessageR.messageCode != OK))
    {
        ptr = NULL;
        return FAILURE;
    }

}

void send_stopControl()
{
printf("system in sendstopcontrol");
    prepControlMessage(&controlMessage, STOP);
    if(0 > sendCtrlMessage(&controlMessage))
    {
        printf("Failed to terminate a connection\n");
        exit(1);
    }
    else 
    {
       printf("Connection Terminated successfully \n");
    }

}

void prepControlMessage(struct Control_Message *cntlMsgPtr, int type)
{
    memset(cntlMsgPtr, 0, sizeof(*cntlMsgPtr));

    /*start of TFRC message: control Message*/
    cntlMsgPtr->messageLength = 0x0E;
    cntlMsgPtr->messageType = CONTROL;
    cntlMsgPtr->messageCode = type;
    cntlMsgPtr->messageCxID = htonl(connectionID);
    cntlMsgPtr->sequenceNumber = rand();
    cntlMsgPtr->messageSize = htons(messageSize);      
}


long nfbTimerUpdate()
{
    static char first_time = RESET;
    if(RESET == first_time)
    {
        noFeedBackTimer = 2000000;  /*Set the alarm to 2 seconds initially*/
        first_time = RESET;
    }
    else
    {
	if (clientParameters->X != 0.0)
	{
        noFeedBackTimer = MAX(4*clientParameters->RTT, ((2*messageSize)/(clientParameters->X)));
        }    
	}
    return noFeedBackTimer;
}


void prepare_DataMessage(struct Data_Message *ptr)
{
    static char firsttime = RESET;
   
    ptr->messageLength = messageSize;
    ptr->messageType = DATA;
    ptr->messageCode = OK;
    ptr->messageCxID = htonl(connectionID);
    
    if(RESET == firsttime)
    {
        ptr->sequenceNumber = (rand());
        firsttime = SET;
    }
    else
    {
        ++(ptr->sequenceNumber);
    }

    gettimeofday(theTime4, NULL);
    usec4 = (theTime4->tv_sec) * 1000000 + (theTime4->tv_usec);
 
    ptr->TimeStamp = (usec4);
    ptr->Sender_RTT = (clientParameters->RTT);
    memset((&(ptr->messageData) + 58), 0xAA, (messageSize-58));
    
}

void clientCNTCCode() {
   gettimeofday(theTime4, NULL);
   usec4 = (theTime4->tv_sec) * 1000000 + (theTime4->tv_usec);
   bStop = 1;
  currentState = STOP;   
  send_stopControl();
  printf("\nTotal Time: %ld Total Packets Sent: %ld Total Bytes sent: %ld Total Acks Received: %ld Total Avg Throughput: %f Avg LossEventRate: %ld\n", usec4-usec2, *numOfPacketSent, (*numOfPacketSent)*messageSize, *numOfAckRecd, (float)((*numOfPacketSent)*messageSize*8)/((usec4-usec2)/1000000), clientParameters->p/*AvgLossRate*/);

exit(0);
}

/*This is the handler for nofeedbacktimer interrupt*/
void nofeedbackTimer_Handler()
{
    timerExpired = SET;
    update_Sendingrate(timerExpired);
    noFeedBackTimer = nfbTimerUpdate();
    timerExpired = RESET;
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int Packet_Drop(double x)
{
    
    if(x>((double)(rand()/RAND_MAX)))
    {
         return 0;
    }
    else
   {
       return 1;
   }
}


void CatchAlarm() 
{

}














