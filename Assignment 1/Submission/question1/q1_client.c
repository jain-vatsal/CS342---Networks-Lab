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

#define MSG_LEN 1000
int main(int argumentsc, char *argumentsv[])
{

    // checking number of args
    if (argumentsc != 3)
    {
        fprintf(stderr, "Usage: %s <Server_IP_Address> <Server_Port_Number>\n", argumentsv[0]);
        return 1;
    }

    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    if (socketClient == -1)
    {
        perror("Unable to create client");
        return 1;
    }

    struct sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(atoi(argumentsv[2]));
    if (inet_pton(AF_INET, argumentsv[1], &sAddr.sin_addr) <= 0)
    {
        perror("Invalid server IP address");
        close(socketClient);
        return 1;
    }

    // checking all the connections
    if (connect(socketClient, (struct sockaddr *)&sAddr, sizeof(sAddr)) == -1)
    {
        perror("Error connecting to server");
        close(socketClient);
        return 1;
    }

    while (1)
    {

        // sending msgs to the server
        char msg[MSG_LEN];
        printf("Enter message to send = ");
        fgets(msg, sizeof(msg), stdin);
        msg[strcspn(msg, "\n")] = '\0'; // Remove newline character

        // Check if the user wants to exit
        if (strcmp(msg, "exit") == 0)
        {
            printf("Exit client");
            break; // Exit the loop
        }

        // Encode using Base64
        unsigned char encData[MSG_LEN * 2];
        int encodedLength = EVP_EncodeBlock(encData, (unsigned char *)msg, strlen(msg));

        // Send to the server
        send(socketClient, encData, encodedLength, 0);

        // Receive acknowledgment from the server
        char acknowledgment[15];
        recv(socketClient, acknowledgment, sizeof(acknowledgment), 0);
        printf("Received acknowledgment from the server: %s\n", acknowledgment);
    }

    close(socketClient);
    return 0;
}
