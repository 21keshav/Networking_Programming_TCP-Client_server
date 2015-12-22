#include    "unp.h"
#define SIZE 1024
      



  int
 main(int argc, char **argv) 
  {  
    int     sockfd,conn;
	int pfd,w,nwrite,n,selready,maxfdv;
    char buf[SIZE];
    char ch;
    struct sockaddr_in servaddr;
    char    sendline[MAXLINE], recvline[MAXLINE];
    fd_set  resetsel;
	struct linger ling;

	
    pfd=atoi(argv[2]);/*recovery of PIPE file discriptor */
    
    strcpy(buf, "Client program started");
	w= write(pfd, buf,SIZE);
	
    if (argc<3)
    { 
		strcpy(buf, "Usage:Client <IP Address or hostname> ?");
		w= write(pfd, buf,SIZE);
		exit(0);
 	}
	
   if((sockfd = Socket(AF_INET, SOCK_STREAM, 0))== -1)/*Socket creation */
    { 
		strcpy(buf, "Socket Error");
        w= write(pfd, buf,SIZE);
	    exit(1);
	}

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(11432);
   Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
   
     conn=connect(sockfd, (SA *) &servaddr, sizeof(servaddr));/*connect to server */
	 if(conn<0)	
	{ 
		strcpy(buf, "Connection Error with Server");
        w= write(pfd, buf,SIZE);
		Close(sockfd);
	    exit(2);
	}
	
	strcpy(buf, "Client has connected to  the Server");
    w= write(pfd, buf,SIZE);
	fflush(stdin);
	printf("Write Data to be Echoed :\n"); 
	
	
    FD_ZERO(&resetsel);
	/* write data to Server and display  echoed data back from server on Client Window*/
	while(1) 
	{
	 
         
		FD_SET(fileno(stdin), &resetsel);
	    FD_SET(sockfd, &resetsel);
	   
	    if(fileno(stdin)>sockfd)
		{
			maxfdv=(fileno(stdin)) + 1;
		}
		else
		{
			maxfdv =(sockfd) + 1;
		}
        
		selready= Select(maxfdv,  &resetsel,  NULL,  NULL,NULL);
		if(selready== -1)
		{
			strcpy(buf, "Select has Failed");
			w= write(pfd, buf,SIZE);
			exit(3);
		}
        if (FD_ISSET(sockfd,  &resetsel)) 
		{  
			if (Read(sockfd, recvline, MAXLINE) == 0)/* Is socket is readable */
            {
				strcpy(buf, " Error-Server has been Terminated");
	            w= write(pfd, buf,SIZE);
				close(sockfd);
				exit(4);
			} 
			strcpy(buf, "Data has been Echoed back from Server");
	        w= write(pfd, buf,SIZE);
            Fputs(recvline, stdout);
         }

        if (FD_ISSET(fileno(stdin), &resetsel)) /* Is input is readable */
		{  
            if (Fgets(sendline, MAXLINE,stdin) == NULL)
            {
	        break;
            }				 
            writeback:
			nwrite= writen(sockfd, sendline, strlen(sendline));
			if (nwrite<0 && errno == EINTR) /*Handling Write Error due to EINTR */
			{ 
				goto writeback;
			} 
			else if (nwrite<0) /*Handling Write Error */
    		{
				strcpy(buf, " Error-Write Error on Socket");
				w= write(pfd, buf,SIZE);
				Close(sockfd);
				exit(5);
			}
        }
    }

	       Close(sockfd);
	       exit(6);
   }	 