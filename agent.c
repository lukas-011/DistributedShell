#include <bits/types/FILE.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include "binToText.h"

#define BUFFER_SMALL 32
#define BUFFER_MID 64
#define BUFFER_LARGE 128
#define BUFFER_GINORMOUS 256000


// CONFIGURATION (Must this be settable at run time?)
#define SAVE_LOCATION "/dshAgentBin" // Will be set to home dir/dshAgentBin (/home/userName/dshAgentBin)


// Global variables
char* saveLocation;

/**
 * The agent will receive the contents of the parallel program, store it on the filesystem, and compile it.
 * @param parallelProg The parallel program
 * @param contentsOfParallelProg The contents of the parallel program
 */
void transfer(const char* parallelProg, char* contentsOfParallelProg) {

    // TODO: What do we save files as? (Name and location?)
    // TODO: Check if directory exists before writing to avoid SEGFAULT
    char* writeLocation = malloc(BUFFER_LARGE);
    printf("%s/%s", saveLocation, parallelProg);
    sprintf( writeLocation,"%s/%s", saveLocation, parallelProg);
    FILE* writeProg = fopen(writeLocation, "w");
    unsigned long programSize = strlen(contentsOfParallelProg)/2; // Size of the program
    char* contentsBin = decodeBinary(contentsOfParallelProg, programSize); // Get the binary
    // Write the contents of programBuffer,
    // of which each element is of size char,
    // of which is the total size programSize
    // to a file writeProg
    fwrite(contentsBin, sizeof(char), programSize, writeProg); // Write to local filesystem
}

/**
 * The agent will run the program using the number n as its single argument. Any output of the compiled program will
 * be returned to the distributed shell.
 * @param parallelProg The parallel program
 * @param n The argument for the program
 */
void run(const char* parallelProg, const char* n) {
    printf("'Run' API Endpoint\n");
}

 /**
  * Parses a request for a parameter's value
  * @param request The request to parse
  * @param param The parameter to find
  * @return The value of the parameter
  */
 char* parseRequest(const char* request, const char* param) {
     char* line = strstr(request, param);
     char* value = malloc(BUFFER_GINORMOUS);

     // Start param length + 1 for equal sign
     int startingPoint = strlen(param)+1;
     for (int i=startingPoint; i<strlen(line); i++) {


        if (line[i] == '&' || line[i] == '\0' || line[i] == '\n') {
            // We found the end in one form or another so let's break outta here
            break;
        }


        value[i-startingPoint] = line[i];
     }
     return value;
 }

/**
 * Listens for requests and calls the required endpoint.
 *
 * @return EXIT_FAILURE if an error occured, or EXIT_SUCCESS if closed gracefully.
 */
int main(void) {
    saveLocation = malloc(BUFFER_LARGE);
    strcpy(saveLocation, getenv(("HOME")));
    strcat(saveLocation, SAVE_LOCATION);
    printf("DSH Agent\n");
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
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof (server_address)) == -1) {
        perror("Socket binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Socket listening failed");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections
    if ((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len)) == -1) {
        perror("Connection acceptance failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected: %s\n", inet_ntoa(client_address.sin_addr));

    // Receive data from the client
    ssize_t bytes_received;
    while ((bytes_received = read(client_socket, buffer, BUFFER_GINORMOUS)) >0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        //printf("Received from client: \n%s", buffer);

        if (strstr(buffer, "POST /transfer")) {
            char* programName = parseRequest(strstr(buffer, "POST /transfer"), "programName");
            char* programBin = parseRequest(strstr(buffer, "POST /transfer"), "programBin");
            transfer(programName, programBin);
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
