#include <iostream>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <bits/stdc++.h>

using namespace std;


 int PORT = 12345;

int wt = 0;
int valid_name = 0;


void receive_messages(int client_socket) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cout << "Server is shut down" << std::endl;
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        

        if(message=="$valid$")
        {
            valid_name = 1;
            wt = 0;
        }
        else if(message == "$invalid$")
        {
                cout<<"User name already exists"<<endl;
            
            valid_name = 0;
            wt = 0;
        }
        else
        {
        std::cout << message << std::endl;
        }
    }
}



int main(int argc,char *prt_num[])
{

    PORT = stoi(prt_num[1]);



    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server details
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to the server's IP address

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Get and send the user's name
    char client_name[10];



    // Start a thread to receive messages
    std::thread(receive_messages, client_socket).detach();

    while(valid_name == 0)
    {
       std::cout << "Enter your name: ";
        cin.getline(client_name, 10);
        // Send the message to the server
        send(client_socket, client_name, strlen(client_name), 0);
        wt = 1;
        while(wt)
        {}  
    }
    

   cout<<"Connected to server , Type a message to broadcast and to send a private message use syntax : \"/private <name of recipient> <message>\" , and to exit type : /exit"<<endl;    



    // Main loop to send messages
    while (true) {
        std::string message;
        std::getline(std::cin, message);

        // Send the message to the server
        send(client_socket, message.c_str(), message.size(), 0);

        // Check for exit command
        if (message == "/exit") {
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
    }

    close(client_socket);
    return 0;
}
