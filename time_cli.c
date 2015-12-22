#include	"unp.h"
#define SIZE 1024
int
main(int argc, char **argv)
{
	int					sockfd, n,connfd;
	char				readdata[MAXLINE + 1];
	struct sockaddr_in	servaddr;
    int pfd,w,conn;
    char buf[SIZE];
	
    pfd=atoi(argv[2]);/*recovery of PIPE file discriptor */
	
   	
	strcpy(buf, "Client Program has started");
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
    servaddr.sin_port = htons(11433);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	
	
	conn=connect(sockfd, (SA *) &servaddr, sizeof(servaddr));/*connect to server */
	 if(conn<0)	
	{ 
		strcpy(buf, "Connection Error with Server");
        w= write(pfd, buf,SIZE);
		Close(sockfd);
	    exit(2);
	}
	strcpy(buf, "Client has connected to the Server");
    w= write(pfd, buf,SIZE);
	
    /* Read Time from Server and display it on Client Window*/
	readback:
	while ( (n = read(sockfd, readdata, MAXLINE)) > 0) 
	{
		readdata[n] = 0;	/* if null terminate */
		strcpy(buf, "Reading Current Time from Server");
        w= write(pfd, buf,SIZE);
		
		printf("Current Date Time is :\n");
		if (fputs(readdata, stdout) == EOF)
		{
			strcpy(buf, "fputs Error");
			w= write(pfd, buf,SIZE);
			Close(sockfd);
			exit(3);
	    }
			
	}		
	
		
	if (n < 0) /*Handling read Error*/
	{	
		strcpy(buf, "Cannot Read from Server");
		w= write(pfd, buf,SIZE);
		exit(4);
    }
	else if (n<0 &&  errno == EINTR)/*Handling Read Error due to EINTR */
    { 
		goto readback;
	}
	else/*Handling Write Error due to Server Termination */
	{
		strcpy(buf, "Error-Server has been Terminated");
		w= write(pfd, buf,SIZE);
		Close(sockfd);
		exit(5);
	}
	
	Close(sockfd);
	exit(6);
}