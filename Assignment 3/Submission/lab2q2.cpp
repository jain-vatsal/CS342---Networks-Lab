#include<bits/stdc++.h> // Include a commonly used header (not standard C++, consider removing)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define CACHE_SIZE 5 // Define a constant for the cache size

struct WebPage {
    char url[1024]; // Character array for URL (consider using C++ string)
    char content[16384]; // Character array for content (consider using C++ string)
    struct WebPage* next; // Pointer to the next WebPage struct
};

struct Cache {
    struct WebPage* head; // Pointer to the head of the cache
    int size; // Size of the cache
};

void insertHead(struct Cache* cache, const char* url, const char* content) {
    struct WebPage* new_page = (struct WebPage*)malloc(sizeof(struct WebPage)); // Allocate memory for a new WebPage
    strncpy(new_page->url, url, sizeof(new_page->url) - 1); // Copy URL into the new WebPage (consider using C++ string)
    strncpy(new_page->content, content, sizeof(new_page->content) - 1); // Copy content into the new WebPage (consider using C++ string)
    new_page->next = cache->head; // Set the next pointer to the current head
    cache->head = new_page; // Update the head of the cache
    cache->size++; // Increase the size of the cache

    // Evict the least recently used page if the cache is full
    if (cache->size > CACHE_SIZE) {
        struct WebPage* prev = NULL;
        struct WebPage* current = cache->head;
        while (current->next != NULL) {
            prev = current;
            current = current->next;
        }
        prev->next = NULL; // Set the previous page's next pointer to NULL
        free(current); // Free the memory of the least recently used page
        cache->size--; // Decrease the size of the cache
    }
}

struct WebPage* existsInCache(struct Cache* cache, const char* url) {
    struct WebPage* current = cache->head;
    struct WebPage* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->url, url) == 0) { // Compare the URL with the given URL
            // Move the accessed page to the front (most recently used)
            if (prev != NULL) {
                prev->next = current->next;
                current->next = cache->head;
                cache->head = current;
            }
            return current; // Return the found WebPage
        }
        prev = current;
        current = current->next;
    }

    return NULL; // Return NULL if the URL is not found in the cache
}

void display_cache(struct Cache* cache) {
    struct WebPage* current = cache->head;
    int c = 1;
    printf("Cache Contents (Most Recently Used to Least Recently Used):\n");
    while (current != NULL) {
        printf("%d. %s\n", c++, current->url); // Print the URL of each WebPage
        current = current->next;
    }
    printf("\nTotal elements in the cache = %d\n\n", c - 1); // Print the total number of elements in the cache
}

int main() {
    struct Cache cache = {NULL, 0}; // Initialize the cache with a null head and size of 0
    int socketClient, check = 0;
    struct sockaddr_in serverAddr;
    struct addrinfo hints, *server;
    char inputURL[1024];

    while (1) {
        printf("Enter the URL of the webpage: ");
        fgets(inputURL, sizeof(inputURL), stdin); // Read input URL from the user
        inputURL[strcspn(inputURL, "\n")] = '\0';  // Remove newline

        check = strcmp(inputURL, "exit"); // Check if the user wants to exit

        if (check == 0) {
            printf("Program terminated.\n");
            break;
        }

        // Initialize the socket
        socketClient = socket(AF_INET, SOCK_STREAM, 0);
        if (socketClient == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Extract the hostname from the URL
        char hostname[1024];
        check = sscanf(inputURL, "http://%1023[^/]/", hostname) ;
        if (check != 1) {
            perror("URL Format is not valid!");
            exit(EXIT_FAILURE);
        }

        // Set up serverAddr using getaddrinfo
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        check = getaddrinfo(hostname, "80", &hints, &server);

        if (check != 0) {
            perror("Error in resolving hostname");
            exit(EXIT_FAILURE);
        }

        memcpy(&serverAddr, server->ai_addr, sizeof(struct sockaddr_in));

        freeaddrinfo(server);  // Free the server info structure

        // HTTP GET request
        struct WebPage* cacheURL = existsInCache(&cache, inputURL);
        if (cacheURL != NULL) {
            printf("\nThe URL %s is ALREADY PRESENT in the cache. NO GET request is needed!! \n\n", inputURL);
            printf("-------------------------------\n\n");
            printf("%s\n", cacheURL->content); // Print the content of the cached page
            printf("-------------------------------\n\n");
        } else {

            printf("\nThe URL %s is NOT PRESENT in the cache. GET request is needed!! \n\n", inputURL);
            // Create an HTTP GET request
            char request[2048];
            snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);

            // Connect to the server

            check = connect(socketClient, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            if (check == -1) {
                perror("Connection to the server failed");
                exit(EXIT_FAILURE);
            }

            // Send the HTTP request
            send(socketClient, request, strlen(request), 0);

            // Receive and store the response
            char res[65536];
            int byteRec = 0;
            while (1) {
                int bytes = recv(socketClient, res + byteRec, sizeof(res) - byteRec, 0);
                if (bytes <= 0) {
                    break;
                }
                byteRec += bytes;
            }

            res[byteRec] = '\0';

            // Insert the retrieved page into the cache
            insertHead(&cache, inputURL, res);

            // Display the retrieved content
            printf("-------------------------------\n\n");
            printf("Page Content:\n\n%s\n\n", res);
            printf("-------------------------------\n\n");
        }

        // Display the cache contents
        display_cache(&cache);
        printf("\n\n");

        // Clean up and close the socket
        close(socketClient);
    }

    return 0;
}
