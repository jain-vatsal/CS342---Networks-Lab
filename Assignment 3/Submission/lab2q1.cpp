#include <stdio.h>      // Include standard input/output library
#include <string.h>     // Include string manipulation library
#include <stdlib.h>     // Include standard library (for functions like malloc, free, etc.)
#include <sys/socket.h> // Include socket-related functions and data structures
#include <arpa/inet.h>  // Include definitions for internet operations
#include <netinet/in.h> // Include internet address family structures and functions
#include <unistd.h>     // Include functions for system calls, like close()
#include<bits/stdc++.h>

// Define an array to store custom DNS servers and a variable to keep track of the count
char custom_dns_servers[10][100];
int custom_dns_server_count = 0;

// Define constants for different DNS record types
#define TYPE_A 1
#define TYPE_NS 2
#define TYPE_CNAME 5
#define TYPE_SOA 6
#define TYPE_PTR 12
#define TYPE_MX 15

// Define a structure to represent a DNS cache entry
struct DNS_CACHE_ENTRY {
    char host[100];        // Store the host name
    unsigned char *rdata;  // Pointer to the resource data
    int ttl;               // Time to live in seconds
    int type;              // Type of the DNS record
};

// Define the size of the DNS cache
#define CACHE_SIZE 100
struct DNS_CACHE_ENTRY dns_cache[CACHE_SIZE]; // Array to store DNS cache entries
int cache_count = 0; // Variable to keep track of the number of entries in the cache

// Function to add an entry to the DNS cache
void add_to_cache(char *host, unsigned char *rdata, int ttl, int type) {
    if (cache_count < CACHE_SIZE) {
        // Copy the host name, set the resource data pointer, TTL, and type
        strcpy(dns_cache[cache_count].host, host);
        dns_cache[cache_count].rdata = rdata;
        dns_cache[cache_count].ttl = ttl;
        dns_cache[cache_count].type = type;
        cache_count++; // Increment the cache count
    }
}

// Function to look up an entry in the DNS cache
unsigned char *lookup_cache(char *host, int *ttl, int *type) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(dns_cache[i].host, host) == 0) {
            *ttl = dns_cache[i].ttl; // Set TTL pointer to TTL value
            *type = dns_cache[i].type; // Set type pointer to DNS record type
            return dns_cache[i].rdata; // Return the resource data pointer
        }
    }
    return NULL; // Return NULL if entry is not found in the cache
}

// Declaration of a function for performing a DNS query
void perform_dns_query(unsigned char *, int);

// Declaration of a function for formatting a DNS name
void format_to_dns_name(unsigned char *, unsigned char *);

// Declaration of a function for reading a DNS name from a packet
unsigned char *read_dns_name(unsigned char *, unsigned char *, int *);

// Declaration of a function for getting custom DNS servers
void get_custom_dns_servers();

// Define a structure to represent a DNS header
struct DNS_HEADER
{
    unsigned short id;        // Identifier for the DNS request
    unsigned char rd : 1;     // Recursion Desired flag
    unsigned char tc : 1;     // Truncation flag
    unsigned char aa : 1;     // Authoritative Answer flag
    unsigned char opcode : 4; // Operation code
    unsigned char qr : 1;     // Query/Response flag
    unsigned char rcode : 4;  // Response code
    unsigned char cd : 1;     // Checking Disabled flag
    unsigned char ad : 1;     // Authenticated Data flag
    unsigned char z : 1;      // Reserved (zero) field
    unsigned char ra : 1;     // Recursion Available flag
    unsigned short q_count;   // Number of questions in the DNS packet
    unsigned short ans_count; // Number of answers in the DNS packet
    unsigned short auth_count; // Number of authoritative records in the DNS packet
    unsigned short add_count; // Number of additional records in the DNS packet
};

// Define a structure to represent a DNS question
struct QUESTION
{
    unsigned short qtype;  // Type of the question
    unsigned short qclass; // Class of the question
};

// Ensure that the following structure is packed tightly (no padding)
#pragma pack(push, 1)
struct RESOURCE_RECORD
{
    unsigned short type;    // Type of the DNS resource record
    unsigned short _class;  // Class of the DNS resource record
    unsigned int ttl;       // Time to live in seconds
    unsigned short data_len; // Length of the resource data
};
#pragma pack(pop)

// Define a structure to represent a DNS resource record
struct RES_RECORD
{
    unsigned char *name;          // Pointer to the resource record's name
    struct RESOURCE_RECORD *resource; // Pointer to the resource record's metadata
    unsigned char *rdata;         // Pointer to the resource record's data
};

// Define a structure to represent a DNS query
typedef struct
{
    unsigned char *name;    // Pointer to the DNS query's name
    struct QUESTION *ques; // Pointer to the DNS query's question
} DNS_QUERY;

int main(int argc, char *argv[])
{
    unsigned char host[100]; // Buffer to store the input host name
    int choice; // Variable to store user's menu choice

    get_custom_dns_servers(); // Retrieve custom DNS servers (function not shown, assumed to be defined elsewhere)

    // Loop for user interaction
    while (true)
    {
        printf("Enter Hostname to Lookup (or enter 0 to exit): ");
        scanf("%s", host);

        if (host[0] == '0')
            break; // Exit loop if user enters '0'

        perform_dns_query(host, TYPE_A); // Perform DNS query for host name
    }

    return 0; // Indicate successful program execution
}



void perform_dns_query(unsigned char *host, int query_type)
{
     // Check if the result is already in the cache
    int ttl, type;
    unsigned char *cached_result = lookup_cache((char *)host, &ttl, &type);

    if (cached_result != NULL) {
         // If result is in the cache, print information and return
        printf("Found in cache. TTL: %d seconds.\n", ttl);

        if (type == TYPE_A) {
            struct in_addr *p = (struct in_addr *)cached_result;
            printf("IPv4 address: %s\n", inet_ntoa(*p));
        } else if (type == TYPE_CNAME) {
            printf("Alias name: %s\n", cached_result);
        }

        return;
    }

    // Initialize variables and data structures
    unsigned char buffer[65536], *qname, *reader;
    int i, j, stop, socket_fd;

    struct sockaddr_in server_address;

    struct RES_RECORD answers[20], auth[20], additional[20];
    struct sockaddr_in destination;

    struct DNS_HEADER *dns = NULL;
    struct QUESTION *question_info = NULL;
    
    // Print the host being resolved
    printf("Resolving %s\n", host);

    // Create a UDP socket for DNS communication
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Set destination DNS server address
    destination.sin_family = AF_INET;
    destination.sin_port = htons(53);
    destination.sin_addr.s_addr = inet_addr("172.17.1.1");
    // Initialize the DNS header
    dns = (struct DNS_HEADER *)&buffer;

    //Setting various fields in the DNS header
    dns->id = (unsigned short)htons(getpid());
    dns->qr = 0;
    dns->opcode = 0;
    dns->aa = 0;
    dns->tc = 0;
    dns->rd = 1;
    dns->ra = 0;
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1);
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    // Set the query name in DNS format
    qname = (unsigned char *)&buffer[sizeof(struct DNS_HEADER)];

    format_to_dns_name(qname, host);

     // Initialize the DNS question
    question_info = (struct QUESTION *)&buffer[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1)];
    //(Setting question type and class
    question_info->qtype = htons(query_type);
    question_info->qclass = htons(1);

    // Sending the DNS query
    printf("\nSending Packet...");
    if (sendto(socket_fd, (char *)buffer, sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr *)&destination, sizeof(destination)) < 0)
    {
        perror("sendto failed");
    }
    printf("Done");

    i = sizeof destination;
    
    // Receiving the DNS response
    printf("\nReceiving answer...");
    if (recvfrom(socket_fd, (char *)buffer, 65536, 0, (struct sockaddr *)&destination, (socklen_t *)&i) < 0)
    {
        perror("recvfrom failed");
    }
    printf("Done");
    // Process the DNS response
    dns = (struct DNS_HEADER *)buffer;
    reader = &buffer[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION)];//Parsing and extracting information from the response
    //Print information about the response
    printf("\nThe response contains : ");
    printf("\n %d Questions.", ntohs(dns->q_count));
    printf("\n %d Answers.", ntohs(dns->ans_count));
    printf("\n %d Authoritative Servers.", ntohs(dns->auth_count));
    printf("\n %d Additional records.\n\n", ntohs(dns->add_count));
    
    stop = 0;
    // Process and print DNS resource records
    for (i = 0; i < ntohs(dns->ans_count); i++)
    {
        answers[i].name = read_dns_name(reader, buffer, &stop);
        reader = reader + stop;

        answers[i].resource = (struct RESOURCE_RECORD *)(reader);
        reader = reader + sizeof(struct RESOURCE_RECORD);

        if (ntohs(answers[i].resource->type) == 1)
        {
            answers[i].rdata = (unsigned char *)malloc(ntohs(answers[i].resource->data_len));

            for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
            {
                answers[i].rdata[j] = reader[j];
            }

            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else
        {
            answers[i].rdata = read_dns_name(reader, buffer, &stop);
            reader = reader + stop;
        }
    }
    //Processing and printing resource record
    for (i = 0; i < ntohs(dns->ans_count); i++)
    {
        printf("Name : %s ", answers[i].name);

        if (ntohs(answers[i].resource->type) == 1)
        {
            struct in_addr *p = (struct in_addr *)answers[i].rdata;
            printf("has IPv4 address : %s\n", inet_ntoa(*p));
        }

        if (ntohs(answers[i].resource->type) == TYPE_CNAME)
        {
            printf("has alias name : %s\n", answers[i].rdata);
        }
    }
    // Add result to cache if TTL is positive
    if (ttl > 0) {
        add_to_cache((char *)host, answers[0].rdata, ttl, ntohs(answers[0].resource->type));
    }

    close(socket_fd);// Close the socket
    return;// Return from the function
}

// Function to format a host name to DNS format
void format_to_dns_name(unsigned char *dns, unsigned char *host)
{
    int lock = 0, i;

    // Append a dot to the host name to ensure proper formatting
    strcat((char *)host, ".");

    for (i = 0; i < strlen((char *)host); i++)
    {
        if (host[i] == '.')
        {
            // Calculate the length of the segment and store it in DNS format
            *dns++ = i - lock;
            // Copy the segment to the DNS format
            for (; lock < i; lock++)
            {
                *dns++ = host[lock];
            }
            lock++;
        }
    }
    *dns++ = '\0'; // End of domain name marker
}

// Function to read a DNS name from a DNS packet
unsigned char *read_dns_name(unsigned char *reader, unsigned char *buffer, int *count)
{
    unsigned char *name;
    unsigned int p = 0, jumped = 0, offset;
    int i, j;

    *count = 1; // Initialize segment count
    name = (unsigned char *)malloc(256); // Allocate memory for DNS name

    name[0] = '\0'; // Initialize DNS name as empty string

    while (*reader != 0)
    {
        if (*reader >= 192) // Check for compression (pointer to offset)
        {
            offset = (*reader) * 256 + *(reader + 1) - 49152; // Calculate offset
            reader = buffer + offset - 1; // Jump to the offset
            jumped = 1; // Set jumped flag
        }
        else
        {
            name[p++] = *reader; // Copy segment character
        }

        reader = reader + 1; // Move to the next character

        if (jumped == 0)
        {
            *count = *count + 1; // Increment segment count
        }
    }

    name[p] = '\0'; // Terminate DNS name

    if (jumped == 1)
    {
        *count = *count + 1; // Increment segment count for compression
    }

    // Convert DNS name to dot-separated format
    for (i = 0; i < (int)strlen((const char *)name); i++)
    {
        p = name[i];
        for (j = 0; j < (int)p; j++)
        {
            name[i] = name[i + 1];
            i = i + 1;
        }
        name[i] = '.';
    }
    name[i - 1] = '\0'; // Remove trailing dot

    return name; // Return formatted DNS name
}


void get_custom_dns_servers()
{
    strcpy(custom_dns_servers[0], "208.67.222.222");
    strcpy(custom_dns_servers[1], "208.67.220.220");
    custom_dns_server_count = 2;
}
// 208.67.222.222