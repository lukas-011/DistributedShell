#include <bits/types/FILE.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include "binToText.h"
#include <pthread.h>

#define BUFFER_SMALL 32
#define BUFFER_MID 64
#define BUFFER_LARGE 128
#define BUFFER_XLARGE 1024
#define BUFFER_GINORMOUS 256000


// CONFIGURATION (Must this be settable at run time?)
#define SAVE_LOCATION "/dshAgentBin" // Will be set to home dir/dshAgentBin (/home/userName/dshAgentBin)

// Prototypes
int sendRequest(char* endpoint, char* sendRequestParam);

// Global variables
char* saveLocation;
struct threadArgs {
    char* programName;
    char* programSrc;
    char* n;
};

// Threads
pthread_t threads[4];
int threadIndexes[4];

/**
 * The agent will receive the contents of the parallel program, store it on the filesystem, and compile it.
 * @param parallelProg The parallel program
 * @param contentsOfParallelProg The contents of the parallel program
 */
void* transfer(void* arg) {
    struct threadArgs* ta = ((struct threadArgs*) arg);
    char* parallelProg = ta->programName;
    char* contentsOfParallelProg = ta->programSrc;

    // TODO: What do we save files as? (Name and location?)
    // TODO: Check if directory exists before writing to avoid SEGFAULT
    char* writeLocation = malloc(BUFFER_LARGE);
    sprintf( writeLocation,"%s/%s", saveLocation, parallelProg);
    FILE* writeFile = fopen(writeLocation, "w");
    unsigned long programLength = strlen(contentsOfParallelProg); // Size of the program
    // Write the contents of programBuffer,
    // of which each element is of size char,
    // of which is the total size programSize
    // to a file writeProg
    fwrite(contentsOfParallelProg, sizeof(char), programLength, writeFile); // Write to local filesystem
    fclose(writeFile);
    return NULL;
}

/**
 * The agent will run the program using the number n as its single argument. Any output of the compiled program will
 * be returned to the distributed shell.
 * @param parallelProg The parallel program
 * @param n The argument for the program
 */
void* run(void* arg) {

    struct threadArgs* ta = ((struct threadArgs*) arg);

    char* parallelProg = ta->programName;
    char* n = ta->n;

    char* compileCommand = calloc(sizeof(char), BUFFER_XLARGE+1);
    strcat(compileCommand, "gcc -o ");
    strcat(compileCommand, saveLocation);
    strcat(compileCommand, "/");
    strcat(compileCommand, parallelProg);
    strcat(compileCommand, " ");
    strcat(compileCommand, saveLocation);
    strcat(compileCommand, "/");
    strcat(compileCommand, parallelProg);
    strcat(compileCommand, ".c");
    strcat(compileCommand, "\0");
    int compileResult = system(compileCommand);

    char* runProg = malloc(BUFFER_XLARGE);
    strcat(runProg, saveLocation);
    strcat(runProg, "/");
    strcat(runProg, parallelProg);
    strcat(runProg, " ");
    strcat(runProg, n);
    int result = system(runProg);

    char* paramToSend = malloc(sizeof(char)*2);
    sprintf(paramToSend,"%d",result);
    sendRequest("m_run_listener",paramToSend);

    return NULL;
}

char* getWordFromString(char* word, int pos) {
    int indexCounter = 0;
    int setChars = 0;
    char* newWord = malloc(BUFFER_XLARGE);
    for (int i=0; i< strlen(word);i++) {
        if (indexCounter == pos-1) {
            setChars = 1;
        }
        if (word[i] == '\0' || word[i] == ' ') {
            indexCounter++;
            if (indexCounter == pos) {
                break;
            }

        }
        if (setChars == 1) {sprintf(newWord, "%s%c", newWord, word[i]);}

    }
    return newWord;
}

char* getEverythingAfter(char* word, int pos) {
    int indexCounter = 0;
    int setChars = 0;
    char* newWord = malloc(BUFFER_GINORMOUS);
    for (int i=0; i< strlen(word);i++) {
        if (indexCounter == pos - 1) {
            setChars = 1;
        }
        if (word[i] == ' ') {
            indexCounter++;
        } else if (word[i] == '\0') {
            break;
        }


        if (setChars == 1) { sprintf(newWord, "%s%c", newWord, word[i]); }

    }

    return newWord;
}

/**
 * Removes trailing \\n from char array.
 *
 * @param charArray Character array to strip \\n from
 *
 * @return The stripped char array
 */
char* stripNewline(char* charArr) {
    for (int i=0; i<BUFFER_MID; i++) {
        if (charArr[i] == '\n') {
            charArr[i] = '\0';
            break;
        }
    }

    return charArr;
}
/**
 * Sends a request to a socket
 * @param requestType The type of request, probably POST
 * @param endpoint The endpoint to hit on the agent
 * @param sendRequestParam The param to be sent.
 * @return DSH_EXIT_SUCCESS if successful, DSH_EXIT_ERROR if not
 */
int sendRequest(char* endpoint, char* sendRequestParam) {

    char* param1 = malloc(BUFFER_LARGE);
    strcpy(param1, sendRequestParam);
    char* request = malloc(BUFFER_GINORMOUS); // 4096

    // Based on client example
    struct sockaddr_in servaddr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8081);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) <0) {
        perror("No connection");
        return 1;
    }
    strcpy(request, endpoint);
    strcat(request, " ");
    strcat(request, param1);

    long connection = write(sockfd, request, strlen(request));
    if (connection < 0) {perror("Couldn't write to socket"); return 1;}

    close(sockfd);

    free(param1);
    free(request);
    return 0;
}

/**
 * Listens for requests and calls the required endpoint.
 *
 * @return EXIT_FAILURE if an error occured, or EXIT_SUCCESS if closed gracefully.
 */
int main(void) {
    int doConnect = 1;
    saveLocation = malloc(BUFFER_LARGE);
    strcpy(saveLocation, getenv(("HOME")));
    strcat(saveLocation, SAVE_LOCATION);
    printf("DSH Agent\n");
    while (doConnect) {

        printf("Waiting for connection...\n");
        int server_socket, client_socket;
        struct sockaddr_in server_address, client_address;
        socklen_t client_address_len = sizeof(client_address);
        char buffer[BUFFER_GINORMOUS];

        // Create socket
        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Initialize server_address structure
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(8080);

        // Bind socket
        if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
            perror("Socket binding failed");
            exit(EXIT_FAILURE);
        }

        // Listen for incoming connections
        if (listen(server_socket, 5) == -1) {
            perror("Socket listening failed");
            exit(EXIT_FAILURE);
        }

        // Accept incoming connections
        if ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len)) == -1) {
            perror("Connection acceptance failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connected: %s\n", inet_ntoa(client_address.sin_addr));

        // Receive data from the client
        ssize_t bytes_received;
        while ((bytes_received = read(client_socket, buffer, BUFFER_GINORMOUS)) > 0) {
            int started = 0;
            int threadToStart;
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("Received:\n-----\n%s", buffer);

            // Determine which thread to start
            for (int i = 0; i < 4; i++) {
                if (i + 1 == 4) {
                    threadToStart = threadIndexes[3];
                } else if (threadIndexes[i + 1] == 0) {
                    threadToStart = threadIndexes[i];
                }
            }

            if (strstr(buffer, "transfer")) {
                started = 1;
                char* programName = stripNewline(getWordFromString(buffer, 2));
                char* programSrc = getEverythingAfter(buffer, 3);

                struct threadArgs ta;
                ta.programName = programName;
                ta.programSrc = programSrc;
                pthread_create(&threads[threadToStart], NULL, (void *) transfer, &ta);

            } else if (strstr(buffer, "run")) {
                started = 1;
                char* parallelProg = stripNewline(getWordFromString(buffer, 2));
                char* n = stripNewline(getWordFromString(buffer, 3));
                struct threadArgs ta;
                ta.programName = parallelProg;
                ta.n = n;
                pthread_create(&threads[threadToStart], NULL, (void *) run, &ta);
            }
            else if (strstr(buffer, "closeAgent")) {
                doConnect = 0;
                printf("Agent will be closed on disconnect.\n");
            }
            if (started == 1) {
                pthread_join(threads[threadToStart], NULL);
            }

        }

        if (bytes_received == -1) {
            perror("Error receiving data");
            exit(EXIT_FAILURE);
        }

        // Close sockets
        close(client_socket);
        close(server_socket);
    }
}