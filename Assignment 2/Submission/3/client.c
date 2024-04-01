// client.c


// importing necessary header files
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>


void main(){

    // variable declaration
    int client,y,x,k=5,m=1,p;

    // buffer for checking ack status
    char buffer[1024];

    // socket declaration and initialization
    struct sockaddr_in servAddr;
    socklen_t addrSize;
    client =socket(PF_INET,SOCK_STREAM,0);
    servAddr.sin_family=AF_INET;
    servAddr.sin_port=htons(5600);
    servAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    memset(servAddr.sin_zero,'\0',sizeof servAddr.sin_zero);

    // connecting to the server
    y = connect(client, (struct sockaddr*)&servAddr,sizeof servAddr);

    if(y==-1){
        printf("Error in connecting \n");
        exit(1);
    }


    // loop till the transmission is complete
    while(k!=0){


        if(m<=5)printf("Sending %d\n",m);

        if(m%2==0)strcpy(buffer,"frame");
        else {

            // showing stop and wait behaviour on odd numbered packets
            strcpy(buffer,"error");
            printf("Packet loss\n");
            for(p=1;p<=3;p++) printf("Waiting for %d seconds\n",p);

            printf("Retransmitting .... \n");
            strcpy(buffer,"frame");
            sleep(3);
        }

        // sending and receiving packets, and acknowledgements
        int x = send(client,buffer,19,0);

        if(x==-1){
            printf("Error in sending\n");
            exit(1);
        }
        else printf("Sent packet %d\n",m);

        int z = recv(client,buffer,1024,0);
        if(z==-1){
            printf("Error in receiving data\n");
            exit(1);
        }

        k--;
        m++;

        if(strncmp(buffer,"ack",3)==0)
        printf("Acknowledgement receieved for %d ... \n",m-1);

        else printf("Ack not received \n");

    }

    close(client);  
}