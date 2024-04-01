// importing the necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    int sClient;
    struct sockaddr_in sAddr;

    // Create socket
    sClient = socket(AF_INET, SOCK_STREAM, 0);
    if (sClient == -1)
    {
        perror("Error creating socket");
        return 1;
    }

    // Setup server address
    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(12345);                  
    sAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    int nRet;
    nRet = connect(sClient, (struct sockaddr *)&sAddr, sizeof(sAddr));
    // Connect
    if (nRet == -1)
    {
        perror("Error connecting to server");
        return 1;
    }

    printf("Connected to server.\n");

    while (1)
    {
        // printf("\033[0;36m"); // Set color to cyan
        // printf("Client: ");
        // printf("\033[0m"); // Reset color
        char buff[1024];
        fgets(buff, sizeof(buff), stdin);

        // Handle graceful exit command
        if (strcmp(buff, "/exit\n") == 0)
        {
            printf("Exiting...\n");
            send(sClient, buff, strlen(buff), 0);
            break;
        }

        send(sClient, buff, strlen(buff), 0);

        // Receive reply from server
        ssize_t bR = recv(sClient, buff, sizeof(buff), 0);
        if (bR <= 0)
        {
            printf("Server was disconnected\n");
            break;
        }

        buff[bR] = '\0';

        // printf("\033[0;33m"); // Set color to yellow
        // printf("Server: %s", buff);
        // printf("\033[0m"); // Reset color
    }

    close(sClient);

    return 0;
}
