#include "unp.h"
#include <ctype.h>
#define SIZE 1024
void serviceselect(int choicesel,char **argv);
void   sig_chld(int signo);
  
int main(int argc, char **argv)
{
    int choicesel,n;
    char cho;	
    if (argc != 2)
	{
		err_quit("Usage: client <IP Address or hostname>?");
	}
	
	while(1)
   {    printf("\nTo Select Service Choose Values\n");
        printf(" 1-Time\n 2-Echo\n 3-Quit\n choice:");
        scanf("%s",&cho);
	    
			switch(cho)
			{
			case '1' :/*Time*/ 
			choicesel=1;
			serviceselect(choicesel,argv);/*call Time service to be implemented*/
			break;
			case '2' :/*Echo*/ 
			choicesel=2;
			serviceselect(choicesel,argv);/*call Echo service to be implemented*/
			break;
			case '3' :/*Quit*/ 
			exit(1); 
			break;
			default:
			choicesel=0;
			printf("\nWrong choice Entered- Please Re-enter choice");
			break;
			}
		
		   
	}
}	





void sig_chld(int signo) /*  SIGCHILD Signal handler Function*/
   {
		pid_t    pidsig;
		int      status;
		while ( (pidsig = waitpid(-1, &status, WNOHANG)) > 0)
		{
			printf("Child  with ID:%d  has been terminated\n", pidsig);
		}
		return; 
	}

                      
    
	 
void serviceselect (int choicesel,char **argv) /* Service select Function*/
{
        
        int pifd[2];
		struct sockaddr_in servaddr;
		struct hostent *hostptr;
		int nread;
        char buf[SIZE];
        char pfdnew[11],ipstr[INET_ADDRSTRLEN],**aptr,*ip;
		pid_t pid;
		bzero(&servaddr, sizeof(servaddr));
	    servaddr.sin_family = AF_INET;
		
		
		
		
        if(inet_pton(AF_INET,argv[1], &servaddr.sin_addr)>0)
        {  
		 
		/* get Host details by address*/
            hostptr = gethostbyaddr(&servaddr.sin_addr, sizeof(servaddr.sin_addr), AF_INET);
            if(hostptr==NULL)
            {
				err_quit("Incorrect <IP Address or hostname>\n");
            }
			ip=argv[1];
			printf("Host address is:%s\n",hostptr->h_name);
			printf("Ip address of host is:%s\n",(char *)ip);
        
		}
       else
        { 
		
		/* get Host details by name*/
        hostptr= gethostbyname (argv[1]);
        if(hostptr==NULL)
        {
			err_quit("Incorrect <IP Address or hostname>\n");
        }
		aptr = hostptr->h_addr_list;
        inet_ntop(hostptr->h_addrtype, *aptr,ipstr, sizeof (ipstr));
        ip=(char *)ipstr;
		printf("Ip address is:%s\n",(char *)ip);
        }
        if (pipe(pifd) == -1)/* Pipe Created*/
  		{
			perror("Pipe has failed\n");
			exit(2);
 		 }
        if ((pid = fork()) < 0) /*Fork Created*/
  		{
			perror("Fork  has failed\n");
			exit(3);
 	    }  
		if(pid==0)
		{
        if ( choicesel == 1 )/*Time Child Process*/
		{
		close(pifd[0]);
		write(pifd[1],"In Time",24);
        sprintf(pfdnew,"%d",pifd[1]);
        execlp("xterm", "xterm", "-e", "./time_cli",ip,pfdnew,(char *) 0);/*Spwan Client program through exec*/
		close(pifd[1]);
		}
		else /*Echo Child Process*/
        {
		close(pifd[0]);
	    write(pifd[1],"In Echo",24);
        sprintf(pfdnew,"%d",pifd[1]);
        execlp("xterm", "xterm", "-e", "./echo_cli",ip,pfdnew,(char *) 0);/*Spwan Client program through exec*/
		close(pifd[1]);
		}
		}
 		else/*Parent Process*/
		{       
        signal (SIGCHLD, sig_chld);
        close(pifd[1]);
		printf("In Parent\n");
		printf("Child ID is:%d\n",pid);
        while((nread = read(pifd[0], buf, SIZE))>0)
    	{
			printf("child: %s\n", buf);
		}	
   	  	close(pifd[0]);
        }
    return;
}
