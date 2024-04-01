// server.c 


//importing the necessary header files
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h> 

void main(){

    // variable declaration
    int server,x,newSock,k=5,m=1,p; // k=5 for the demo on 5 packets only, m indicates packet number

    // buffer for checking ack status
    char buffer[1024];

    // socket declaration and initialization
    struct sockaddr_in servAddr;
    struct sockaddr_storage store;
    socklen_t addrSize;
    server = socket(PF_INET,SOCK_STREAM,0);
    servAddr.sin_family=AF_INET;
    servAddr.sin_port=htons(5600);
    servAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    memset(servAddr.sin_zero,'\0',sizeof servAddr.sin_zero);

    // binding the socket to the port
    bind(server,(struct sockaddr*)&servAddr,sizeof(servAddr));

    // listening at the port
    if(listen(server,5)==0)printf("Listening\n");
    else printf("Error\n");

    // accepting new connections
    newSock=accept(server,(struct sockaddr*)&store,&addrSize);

    if(newSock==-1){
        printf("Error in creating socket\n");
        exit(1);
    }

    // loop till the transmission is complete
    while(k!=0){

        // server acting as receiver
        int y = recv(newSock,buffer,1024,0);


        if(y==-1){
            printf("Error in receiving\n");exit(1);
        }

        // successfully receiving packet
        if(strncmp(buffer,"frame",5)==0)printf("Received packet %d successfully\n",m);
        else printf("Not received packet %d \n",m);

        if(m%2==0)strcpy(buffer,"ack");
        else {

            // showing stop and wait behaviour on odd numbered packets
            strcpy(buffer,"kca");
            printf("Packet lost\n");
            for(p=1;p<=3;p++)printf("Waiting for %d seconds\n",p);

            printf("Retransmitting ack..\n");
            strcpy(buffer,"ack");
            sleep(3);
        }


        // sending acknowledgement and further transmissions
        printf("Sending ack %d ...\n",m);
        int z = send(newSock,buffer,19,0);
        if(z==-1){
            printf("Error in sending \n");
            exit(1);
        }
        k--;
        m++;
    }
    close(newSock);
}