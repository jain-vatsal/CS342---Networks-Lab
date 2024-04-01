
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <bits/stdc++.h>

using namespace std;



const int MAX_CLIENTS = 1000;
int PORT = 12345;

std::vector<int> client_sockets;
std::vector<std::string> client_names;
//vector<int> active;


void display(vector<string> &s , vector<int> &a)
{
    for(int i=0;i<s.size();i++)
    {
        cout<<s[i]<<" -> "<<a[i]<<endl;
    }
    
}


const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s)
{
    return rtrim(ltrim(s));
}

void broadcast(int sender_index, const std::string &message)
{
    for (int i = 0; i < client_sockets.size(); ++i)
    {
        if (i != sender_index)
        {
            send(client_sockets[i], message.c_str(), message.size(), 0);
        }
    }
}



void handle_client(int client_socket, int client_index)
{
    
  //  display(client_names,client_sockets);

    char buffer[1024];
    while (true)
    {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0)
        {
           
            std::cout << "User : " << client_names[client_index] << " is disconnected" << std::endl;

          broadcast(client_index, client_names[client_index] + ", has left the chat.");

           
            int n = client_names.size();

            for(int i= client_index ; i<n-1;i++)
            {
               client_names[i] = client_names[i+1];
               client_sockets[i] = client_sockets[i+1];
            }
            client_names.pop_back();
            client_sockets.pop_back();

        //    active[client_index] = 0;


            close(client_socket);
            
  //  display(client_names,client_sockets);


            break;
        }
        buffer[bytes_received] = '\0';
        std::string message(buffer);

        // Check for exit command

        if (message == "/exit")
        {
            std::cout << "Client : " << client_names[client_index] << " is disconnected" << std::endl;
          broadcast(client_index, client_names[client_index] + ", has left the chat.");


            int n = client_names.size();

            for(int i= client_index ; i<n-1;i++)
            {
               client_names[i] = client_names[i+1];
               client_sockets[i] = client_sockets[i+1];
            }
            client_names.pop_back();
            client_sockets.pop_back();
       //     active[client_index] = 0;

            close(client_socket);
 //  display(client_names,client_sockets);


            break;
        }

        // Check for private message
        if (message.substr(0, 8) == "/private")
        {
            std::string recipient_name = message.substr(9, message.find(' ', 9) - 8);
            recipient_name = trim(recipient_name);
          
          
            std::string private_message = message.substr(message.find(' ', 9) + 1);
         
         
            int brk = 0;

            for (int i = 0; i < client_names.size(); ++i)
            {
                if (client_names[i] == recipient_name )
                {
                    private_message = "private message from " + client_names[client_index] + " : " + private_message;
                    
                    send(client_sockets[i], private_message.c_str(), private_message.size(), 0);

                    brk = 1;
                    break;
                }
            }
            if (brk == 0)
            {

                string msg = "Sorry, user not found.";
            send(client_sockets[client_index], msg.c_str(), msg.size(), 0);
            }

        }
        else
        {
            // Broadcast the message to all clients
            broadcast(client_index, client_names[client_index] + ": " + message);
        }
    }
}


int ending = 0;

void Read()
{
    while(true)
    {
    string s;
    cin>>s;
    //cout<<"input :- "<<s<<endl;
    if(s=="/exit")
    {
        ending = 1;
       // cout<<ending<<endl;
        exit(0);
    }
    }
}


bool check(vector<string> &names, string name)
{
    for (int i = 0; i < names.size(); i++)
    {
        if (names[i] == name )
        {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------MAIN---------------------------------------------------------------------------
int main(int argc, char *prt_num[])
{

    PORT = stoi(prt_num[1]);


    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("Listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    //int cnt = 0;
       cout<<"To shut down server type: /exit"<<endl;
        std::cout << "Server is listening..." << std::endl;

    thread to_end(Read);


    while (true)
    {
        if(ending)
        {
         close(server_socket);
         return 0;
        } 

        // Accept a new connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1)
        {
            perror("Acceptance failed");
            continue;
        }
        else
        {

            // Get and store the client's name


            string client_NAME;

            while(true)
            {

            
            char client_name_buffer[10];
            memset(client_name_buffer, ' ', sizeof(client_name_buffer));

            recv(client_socket, client_name_buffer, sizeof(client_name_buffer), 0);

            string client_name(client_name_buffer);

            client_name = trim(client_name);


            
            if (check(client_names, client_name))
            {
                string temp = "$invalid$";

                send(client_socket, temp.c_str(),temp.size(), 0);
                
                continue;
            }
            else
            {
                string temp = "$valid$";

                send(client_socket, temp.c_str(),temp.size(), 0);

                client_NAME = client_name;

                break;
            }
             
            }



            client_names.push_back(client_NAME);

            cout << "User : " + client_NAME + " , is been added to server." << endl;

            // Add client socket to the vector
            client_sockets.push_back(client_socket);

            // Start a new thread to handle the client
        //    active.push_back(1);
            std::thread(handle_client, client_socket, client_sockets.size() - 1).detach();

            // Notify all clients of the new user
            broadcast(client_sockets.size() - 1,  client_NAME + ", has joined the chat.");
        }
    }

    close(server_socket);
    return 0;
}