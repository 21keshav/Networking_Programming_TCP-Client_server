#include     "unpthread.h"
#include<stdio.h>
#include<time.h>


static void *echosel(void *arg);
static void *timesel(void *arg);
  
 int main(int argc, char **argv)
  {  
	int  echosockfd,timesockfd ,connfd,*conptr,pollready,selready;
	int rset;
	pthread_t th;
    socklen_t clilen,len;
    struct sockaddr_in cliaddr, servaddr;
    struct pollfd client[2];
	int fileflags;
	char str[INET_ADDRSTRLEN];
	  
     rset=1;// for setsockopt() to select SO_REUSEADr 
	   
	if((timesockfd = socket(AF_INET, SOCK_STREAM, 0))== -1)/* Time Socket creation */
	{
		err_quit(" Time socket Error\n");
	}
	if((echosockfd = socket(AF_INET, SOCK_STREAM, 0))== -1)/* Echo Socket creation */
	{
		err_quit(" Echo socket Error\n");
	}
	
    setsockopt(timesockfd, SOL_SOCKET, SO_REUSEADDR,&rset, sizeof(int));
	setsockopt(echosockfd, SOL_SOCKET, SO_REUSEADDR,&rset, sizeof(int));
	 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
     
	servaddr.sin_port = htons (11433);
    Bind(timesockfd, (SA *) &servaddr, sizeof(servaddr));/*Bind Time Socket*/
   	Listen(timesockfd, LISTENQ);/*Listen for Time clients*/
	 
	servaddr.sin_port = htons (11432);
    Bind(echosockfd, (SA *) &servaddr, sizeof(servaddr));/*Bind Time Socket*/
    Listen(echosockfd, LISTENQ);/*Listen for Echo clients*/
	 
	signal(SIGPIPE,SIG_IGN);/*Handling SIGPIPE */ 
    Fputs("Server Waiting For Connections.....\n", stdout);
    for(;;)
	{
		 
		client[0].fd =timesockfd ;
		client[0].events =POLLRDNORM; 

		client[1].fd =echosockfd ;
		client[1].events =POLLRDNORM;
		  
		pollready=poll(client,3,NULL);
		if(pollready== -1)
		{
			err_sys("Poll has failed");
		}
		if (client[0].revents & POLLRDNORM)
		{
			conptr = Malloc(sizeof(int));
			*conptr = Accept(timesockfd, (SA *) NULL,NULL);/*Accept connection form time client*/
			Fputs("Connection formed between a Time client with the server\n",stdout);
			Pthread_create(&th, NULL, &timesel, conptr);/*creation of Thread*/
		}
		 
	    if (client[1].revents & POLLRDNORM)
		{
			clilen = sizeof(cliaddr);
			conptr = Malloc(sizeof(int));
			*conptr = Accept(echosockfd, (SA *) &cliaddr, &clilen);/*Accept connection form Echo client*/
			Fputs("Connection formed between a Echo client with the server\n",stdout);
			Pthread_create(&th, NULL, &echosel, conptr);/*creation of Thread*/
		 
		}
	}
 }
	

	static void *timesel(void *arg)/* Time Thread*/
    {
        int     connfd,maxifdp,nwrite,selready,nread,fileflags;
		char				buff[MAXLINE],receivedata[MAXLINE];
        time_t				ticks;
        struct timeval timeinv;
		fd_set resetsel;
		
		connfd = *((int *) arg);
		free(arg);
		
        Pthread_detach(pthread_self());/* Detach Thread*/
		 
	    FD_ZERO(&resetsel);
		
        while(1)
		{
			ticks = time(NULL);
			snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
			nwrite=write(connfd, buff, strlen(buff));/* Write time to client*/
		
			if(nwrite<0&& errno == EPIPE)	/*Handling Write Error due to EPIPE */	
			{
				Fputs("Time Client termination:EPIPE error detected\n", stdout); 
				Close(connfd);
				return;
			}
			else if (nwrite<0 && errno == EINTR)/*Handling Write Error due to EINTR */	
			{ 
				continue;
			} 
			else if (nwrite<0)/*Handling Write Error */	
			{
				Fputs(" Time Client termination:Write Error Detected,error occured due to\n", stdout); 
				Fputs(strerror(errno),stdout);  
				Fputs("\n",stdout);
				Close(connfd);
				return;
			}
				
			FD_SET(connfd, &resetsel);
			timeinv.tv_sec = 5;
			timeinv.tv_usec = 0;
			maxifdp = connfd +  1;
			selready= select(maxifdp,  &resetsel,  NULL,  NULL,&timeinv);/*sleeping for 5 secs using select*/
			if(selready== -1)
			{
				err_sys("Select has failed\n");
			}

			if (FD_ISSET(connfd,  &resetsel))/* is Connection readable */
			{  
				if ( (nread=Read(connfd, receivedata, MAXLINE)) == 0)/*Handling Client Termination  */	
                { 
					Fputs(" Time Client termination:socket read returned with value 0\n", stdout); 
					Close(connfd);
					return;
                }
				else if(nread<0) /*Handling read Error */	
				{
				    Fputs("Time Client termination:socket read returned with value -1,error occured due to\n", stdout); 
					Fputs(strerror(errno),stdout);  
					Fputs("\n",stdout);
					Close(connfd);
					return;
		 		}
			}
         	 
	    }

		 
			Close(connfd);             
			return (NULL);

        
    }
	static void *echosel(void *arg)/* Echo Thread*/
    {
        int     connfd,wr,fileflags;
		ssize_t n;
        char    buf[MAXLINE];
        connfd = *((int *) arg);
        free(arg);
		
        Pthread_detach(pthread_self());/* Detach Thread*/
         

		back:
		while ( (n = readline(connfd, buf, MAXLINE)) > 0)/*Read data from Client*/
		{ 
			wrback:
		  wr= writen(connfd, buf, n);/*write back data from Client*/
		}	
		  if(wr<0&& errno == EPIPE)		/*Handling Write Error due to EPIPE */	
		{
			Fputs(" Echo Client termination:EPIPE error detected\n", stdout); 
			Close(connfd);
			return;
		}
		else if (wr<0 && errno == EINTR)/*Handling Write Error due to EINTR */	
		{ 
			goto wrback;
		} 
		else if (wr<0)/*Handling Write Error */	
		{
			Fputs("Echo Client termination:Write error detected,error occured due to\n", stdout); 
			Fputs(strerror(errno),stdout);  
			Fputs("\n",stdout);
			Close(connfd);
			return;
		}
				
						
        if (n < 0 && errno == EINTR)/*Handling read Error due to EINTR */	
        { 
			goto back;
	    }
		else if (n < 0)
        {   /*Handling read Error */
			Fputs(" Echo Client termination:socket read returned with value -1, error occured due to\n", stdout);
            Fputs(strerror(errno),stdout);
            Fputs("\n",stdout);   			
			Close(connfd);
			return;
		}
		else	 /*Handling client Termination */	
		{
			Fputs(" Echo Client termination:socket read returned with value 0\n", stdout); 
			Close(connfd);
			return;
		}
          
			Close(connfd);               
			return (NULL);
    
    }