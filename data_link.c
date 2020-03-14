#include <stdio.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 255
#define MAXPENDING 5

void err_sys(char *mess) {perror(mess); exit(1);}
void delay(int  numsec){
	int milli_seconds=1000*numsec;
	clock_t start_time=clock();
	while(clock()<start_time+milli_seconds);
}
void handle_client(int sock, FILE *fptr,int sock1){
        //CREATE DATE AND TIME
	char buffer[BUFFSIZE];
        int hours, minutes, seconds, day, month, year;
        time_t now;
        time(&now);
        struct tm *local=localtime(&now);
        hours=local->tm_hour;
        minutes=local->tm_min;
        seconds=local->tm_sec;
        day=local->tm_mday;
        month=local->tm_mon+1;
        year=local->tm_year+1900;
        time_t t;
        int number; int wordlen;
        srand((unsigned)time(&t));
        //READING NUM OF REPS
        while(1){
                read(sock,&number, sizeof(number));
                if(number==0) break;
                //Reading length word
                read(sock,&wordlen,sizeof(wordlen));
		write(sock1,&wordlen,sizeof(wordlen));
                //Generating X random numbers from 0 to 10
                for(int i=1;i<=number;i++){
                        read(sock,&buffer[0],wordlen); //LLEGIM LA PARAULA
                        if(hours<12) fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] LINE %d: %s \n", day, month, year, hours, minutes, seconds, i, buffer);
                        else fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] LINE %d: %s \n", day, month, year, hours-12, minutes, seconds, i, buffer);
                        if (hours<12) fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] INPUT %d: %s \n", day, month, year, hours, minutes, seconds, i, buffer);
                        else fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] INPUT %d: %s \n", day, month, year, hours-12, minutes, seconds, i, buffer);
                        delay(rand()%10); //DELAY ALEATORI DE 0-9s
                        write(sock1,buffer,BUFFSIZE); //POOOT FALLAR (ENVIEM LA PARAULA A LA BS)
                        if(hours<12) fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] OUTPUT %d: %s \n", day, month, year, hours, minutes, seconds, i, buffer);
                        else fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] OUTPUT %d: %s \n", day, month, year, hours-12, minutes, seconds, i, buffer);
                }

        }
}



int main(int argc, char*argv[]){
	struct sockaddr_in echoserver1,echoserver2,echoclient;
        int serversock, clientsock;
	char buffer[BUFFSIZE];
        unsigned int echolen;
        int sock,result;
	int received=0;

	if(argc!=4){
                fprintf(stderr,"Usage: %s <logfilename> <ip_Server> <port>\n", argv[0]);
                exit(1);
        }
	//CREATING SOCKET
	sock=socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
        if(sock<0){
                err_sys("Error socket");
        }
	//SET INFORMATION FOR sockaddr_in
        memset(&echoserver1,0,sizeof(echoserver1)); //reset memory
        echoserver1.sin_family=AF_INET; //internet/ip
        echoserver1.sin_addr.s_addr=inet_addr(argv[2]); //ip adress
        echoserver1.sin_port=htons(atoi(argv[3])); //server port
	//TYING CONNECTION WITH THE SERVER
        result=connect(sock, (struct sockaddr *) &echoserver1, sizeof(echoserver1));
        if(result<0){
                err_sys("Error connect");
        }

	/* Create TCP socket */
    	serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    	if (serversock < 0) {
        err_sys("Error socket");
    	}

    	/* Set information for sockaddr_in structure */
    	memset(&echoserver2, 0, sizeof(echoserver2));       /* we reset memory */
    	echoserver2.sin_family = AF_INET;                  /* Internet/IP */
    	echoserver2.sin_addr.s_addr = htonl(INADDR_ANY);   /* ANY address */
    	echoserver2.sin_port = htons(8081);       /* server port */

    	/* Bind socket */
    	result = bind(serversock, (struct sockaddr *) &echoserver2, sizeof(echoserver2));
    	if (result < 0) {
       		err_sys("Error bind");
    	}

    	/* Listen socket */
    	result = listen(serversock, MAXPENDING);
    	if (result < 0) {
        	err_sys("Error listen");
    	}
	//CREATING FILE WITH THE INTRODUCED FILENAME
	FILE *fptr=NULL;
	strcat(argv[1],".txt");
	fptr=fopen(argv[1],"w");

	while(1){
		unsigned int clientlen = sizeof(echoclient);
        	/* Wait for a connection from a client */
        	clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen);
		if(clientsock<0){
                        err_sys("Error accept");
                }
                fprintf(stdout, "Client: %s\n", inet_ntoa(echoclient.sin_addr));
		/* Call function to handle socket */
                handle_client(clientsock,fptr,sock);
		close(sock);
		exit(0);
	}
}
