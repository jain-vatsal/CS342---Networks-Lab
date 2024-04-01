#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_NODES 50
#define MAX_BACKOFF 20

int successful_transmissions = 0;
int collisions = 0;
int backoffs = 0;

typedef struct Node {
    int id;
    bool transmitting;
    int backoff_counter;
    bool done;
} Node;

Node nodes[MAX_NODES];

void initializeNodes(int N) {
    for (int i = 0; i < N; i++) {
        nodes[i].id = i;
        nodes[i].transmitting = false;
        nodes[i].backoff_counter = rand() % MAX_BACKOFF;
        nodes[i].done = false;
    }
}

bool isChannelClear() {
    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].transmitting)
            return false;
    }
    return true;
}

void backoff(Node* node) {
    node->backoff_counter = rand() % MAX_BACKOFF;
    backoffs++;
}

void transmit(Node* node) {
    node->transmitting = true;
    successful_transmissions++;
    printf("Node %d transmits successfully\n", node->id);
}

void simulate(int N, int rounds) {
    for (int r = 0; r < rounds; r++) {
        int tag = 0;
        for (int i = 0; i < N; i++) {
            if(nodes[i].done == true) continue;
            else if (isChannelClear()) {
                if (nodes[i].backoff_counter == 0) {
                    transmit(&nodes[i]);
                    tag = 1;
                } else {
                    nodes[i].backoff_counter--;
                }
            }
            else if (nodes[i].transmitting) {
                nodes[i].transmitting = false;
                nodes[i].backoff_counter = 0;
                nodes[i].done = true;
                printf("Node %d transmits Done\n", nodes[i].id);
                tag = 1;
            }
            else if(!isChannelClear() && nodes[i].backoff_counter == 0){
                collisions++;
                backoff(&nodes[i]);
                printf("Node %d Collides and backoff \n", nodes[i].id);
                tag = 1;
            }
        }
        // if(tag == 0) printf("Nothing happened in this round \n");
    }
}

int main() {
    srand(time(NULL));

    int N = 50; // Number of nodes
    int rounds = 150; // Number of simulation rounds

    initializeNodes(N);
    simulate(N, rounds);

    printf("\nSimulation Results:\n");
    printf("Successful Transmissions: %d\n", successful_transmissions);
    printf("Collisions: %d\n", collisions);
    printf("Backoffs: %d\n", backoffs);

    return 0;
}