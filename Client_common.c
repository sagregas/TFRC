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

#include "Global.h"
#include "TFRC_Client.h"
#include "Client_common.h"

/*This function updates the global client parametere everytime an acknowledgement is received*/
void update_Parameters(struct Ack_Message *ptr)
{
     static char firstTime = RESET;
     long usec;
     struct timeval curTime;

     (void) gettimeofday (&curTime, (struct timezone *) NULL);
     usec =  ((curTime.tv_sec *1000000) + curTime.tv_usec);
     clientParameters->p = DEFLOOR((double)(ptr->lossEventRate));
     clientParameters->x_Recd = DEFLOOR((double)(ptr->receiveRate));
     clientParameters->b = 1;
     clientParameters->t_now = usec;
    /*calculate a new RTT*/
   
    clientParameters->R_sample = (usec - (ptr->TimeStamp) ) - ((ptr->t_delay));
  
    if(RESET == firstTime)
    {
        clientParameters->RTT = clientParameters->R_sample;
        firstTime = SET;
    }
    else
    {
        clientParameters->RTT = (long)(((filterConstant_Q * ((float)clientParameters->RTT)) + ((1 - filterConstant_Q)*(float)clientParameters->R_sample)));
    }

    clientParameters->t_RTO = 4*(clientParameters->RTT);
  
    update_Sendingrate(timerExpired);
}


/*This function updates th sending rate when ack is recieved or when feedback timer is popped*/
void update_Sendingrate(char nofbtimerElapsed)
{
    double temp;
    double temp1;
    double temp2;
    double temp3;
    if(nofbtimerElapsed == RESET)
    {
        if((clientParameters->p) > 0.0)
        {
	    temp = (clientParameters->RTT)*sqrt(2*(clientParameters->b)*(clientParameters->p)/3);
	    temp1 = (clientParameters->t_RTO) * (3*sqrt(3*(clientParameters->b)*(clientParameters->p)/8));
	    temp2 = 1 + (32*((clientParameters->p)*(clientParameters->p)));
	    temp3 = temp + (temp1 * (clientParameters->p) * temp2);
            if(temp3 != 0.0)
            {
                clientParameters->X_calc = (double)((messageSize*8)/temp3);
            }
            
            
                    temp = MIN(clientParameters->X_calc, 2*(clientParameters->x_Recd));
                 
                    clientParameters->X = MAX(temp, ((double)(messageSize*8)/64));
                
        }
        else
        {
          
            if((clientParameters->t_now) - (clientParameters->tld)  >= (clientParameters->RTT))
            {   
		if ( clientParameters->RTT != 0.0)
		{
                    clientParameters->X = MAX(MIN((2*(clientParameters->X)), 2*(clientParameters->x_Recd)), (messageSize*8/(clientParameters->RTT)));
		}              
 		 clientParameters->tld = clientParameters->t_now;         
            }
        }
    }
    else
    {
          if(clientParameters->X_calc > (2*clientParameters->x_Recd))
          {
              clientParameters->x_Recd = MAX(clientParameters->x_Recd/2, messageSize*8/128); 
          }
          else
          {
              clientParameters->x_Recd = (clientParameters->X_calc/4);
          } 
          clientParameters->X = MAX((clientParameters->X)/2, (messageSize*8/64));     
    }
   /*limit the current sending rate to the maximum allowed bandwidth*/
    if(maxAllowedThrough < ((clientParameters->X)*1000000))
    {
        clientParameters->X = maxAllowedThrough;
    }

}




  










