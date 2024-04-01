// importing the necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <pthread.h>

void *handleClients(void *cPtr)
{
    int sClient = *((int *)cPtr);
    char buf[1024];

    while (1)
    {
        int bR = recv(sClient, buf, sizeof(buf), 0);
        if (bR <= 0)
        {
            printf("Client %d disconnected.\n", sClient);
            break;
        }

        buf[bR] = '\0';

        // Handle graceful exit command
        int r = strcmp(buf, "/exit\n");
        if (r == 0)
        {
            printf("Client %d wanted to exit.\n", sClient);
            send(sClient, "Server: Goodbye!\n", 18, 0);
            break;
        }

        // printing the messages
        printf("Client %d: %s", sClient, buf);

        // Send reply to client
        printf("Server: ");
        fgets(buf, sizeof(buf), stdin);
        send(sClient, buf, strlen(buf), 0);
    }

    close(sClient);
    free(cPtr);
    return NULL;
}

int main()
{
    int sSocket;
    struct sockaddr_in sAddr;

    // Create socket
    sSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (sSocket == -1)
    {
        perror("Error creating socket");
        return 1;
    }

    // Setup server address
    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(12345); // Use any available port
    sAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    int nRet;
    nRet = bind(sSocket, (struct sockaddr *)&sAddr, sizeof(sAddr));
    if (nRet == -1)
    {
        perror("Error binding");
        return 1;
    }

    // Listen
    nRet = listen(sSocket, 5);
    if (nRet == -1)
    {
        perror("Error listening");
        return 1;
    }

    printf("Server is listening for incoming connections.\n");

    while (1)
    {
        // Accept connection
        int *cPtr = (int *)malloc(sizeof(int));
        *cPtr = accept(sSocket, NULL, NULL);
        if (*cPtr == -1)
        {
            perror("Unable to connect to client");
            continue;
        }

        printf("Client connected.\n");

        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClients, cPtr) != 0)
        {
            perror("Unable to create thread");
            close(*cPtr);
            free(cPtr);
        }
    }

    close(sSocket);

    return 0;
}
