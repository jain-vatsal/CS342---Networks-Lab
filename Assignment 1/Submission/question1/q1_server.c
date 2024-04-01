// importing all the necessary header files
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// these are imported for the necessary conversions into base 64 etc.
#include <arpa/inet.h>
#include <openssl/evp.h>

// pre-defined msg length
#define msgLength 1000

void *handleClient(void *arguments)
{
    int socketClient = *((int *)arguments);

    // Receive and process msgs from the client
    while (1)
    {

        // declaring and initializing character array
        char msg[msgLength];
        memset(msg, 0, sizeof(msg));

        // receiving messages from the client
        int nRet;
        nRet = recv(socketClient, msg, sizeof(msg), 0);
        if (nRet <= 0)
        {
            perror("Error receiving data from client");
            break;
        }

        // decoding
        unsigned char decodedString[msgLength];
        int dLen = EVP_DecodeBlock(decodedString, (unsigned char *)msg, nRet);

        if (dLen > 0)
        {
            decodedString[dLen] = '\0';
            printf("Actual message from client: %s\n", decodedString);
        }

        // Send an acknowledgment back to the client
        send(socketClient, "Acknowledgment from the server", 14, 0);
    }

    close(socketClient);
    pthread_exit(NULL);
}

int main(int argumentsc, char *argumentsv[])
{
    // checking number of args.
    if (argumentsc != 2)
    {
        fprintf(stderr, "Usage: %s <Server_Port_Number>\n", argumentsv[0]);
        return 1;
    }

    // creating the server socket
    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServer == -1)
    {
        perror("Not able to create server socket");
        return 1;
    }

    // initializing socket variables
    struct sockaddr_in srv;
    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(atoi(argumentsv[1]));

    int nRet;
    nRet = bind(socketServer, (struct sockaddr *)&srv, sizeof(srv));
    if (nRet == -1)
    {
        perror("Unable to bind to server socket");
        close(socketServer);
        return 1;
    }

    nRet = listen(socketServer, 5);
    if (nRet == -1)
    {
        perror("Unable to listen to server socket");
        close(socketServer);
        return 1;
    }

    printf("Server is listening on port %s...\n", argumentsv[1]);

    while (1)
    {
        struct sockaddr_in cAddr;
        socklen_t cAddr_size = sizeof(cAddr);
        int socketClient = accept(socketServer, (struct sockaddr *)&cAddr, &cAddr_size);
        if (socketClient == -1)
        {
            perror("Error accepting client connection");
            continue;
        }

        pthread_t thread;
        pthread_create(&thread, NULL, handleClient, &socketClient);
        pthread_detach(thread);
    }

    close(socketServer);
    return 0;
}
