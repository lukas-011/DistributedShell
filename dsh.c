#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "constants.h"
#include "binToText.h"

// We can use different buffer sizes to save memory
#define SMALL_BUFFER 32
#define MID_BUFFER 64
#define CMD_BUFFER 128
#define MAX_BUFFER 128
#define GINORMOUS_BUFFER 4096

struct Argument {
    char* argument;
};

/**
 * Initialized global structs for Argument
 */
struct Argument Arguments[8] = {
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {}
};

struct PathVar {
    char* DirectoryName;
};

/**
 * Initialized global structs for PathVar
 */
struct PathVar PathVars[18] = {
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {}
};

struct MAgent {
    char* ip;
    char* port;
};

/**
 * Initialized global structs for m_agent
 */
struct MAgent MAgents[32] = {
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {}
};

struct MCP {
    char* local;
    char* dest;
};

struct MRun {
    char* mainProg;
    char* parellelProg;
};

struct sendRequestParam {
    const char* programName;
    const char* programSrc;
    const char* n;
    const char* ip;
    const int port;
};


// Program function prototypes
int startProc(char* procName);
void exitShell();
int doCommand(char* cmd);
int doMAgent();
int doMCp();
int doMRun();
void initialize();

// Helper function prototypes
char* stripNewline(char* charArr);
void separateArguments(const char* args);
char* setStructForArgumentsPATH_VAR(char* charArr);
int sendProgram(const char* programName, FILE* program, const char* ip, const int port);
unsigned long getProgramSize(FILE* program);
int sendRequest(char* requestType, char* endpoint, struct sendRequestParam reqParams);
char* saveProgramToBuffer(FILE* program, unsigned long programSize);
char* readTextToBuffer(FILE* program);
void* waitForResponsFromAgents(void* arg);
char* getWordFromString(char* word, int pos);
void* waitForResponseFromAgents(void* args);

// Exit codes
enum ExitCode {
    DSH_EXIT_SUCCESS,
    DSH_EXIT_ERROR
};

//**********************************************************************************************************************
/**
 * Program execution starts here
 *
 * @return 9 If we somehow break from the while loop.
 */
int main() {
    initialize();
    printf("%s",STR_GREETING);

    while(1) {
        enum ExitCode result;
        char userInput[CMD_BUFFER];
        printf(">");
        fgets(userInput, CMD_BUFFER, stdin);
        result = doCommand(userInput);
        if (result == DSH_EXIT_ERROR) {
            printf(STR_FAILSTART, userInput);
        }
    }
}

//**********************************************************************************************************************
/**
 * Initialize defaults for the program
 */
void initialize() {
    // Set PATH struct
    setStructForArgumentsPATH_VAR(getenv(STR_PATH));
}

//**********************************************************************************************************************
/**
 * Determines which method to call based on the command the user inputs
 *
 * @param cmd Command to run
 *
 * @return DSH_EXIT_SUCCESS if doing command was successful; DSH_EXIT_ERROR if failed
 */
int doCommand(char* cmd) {
    // Remove new line from command
    enum ExitCode result = DSH_EXIT_SUCCESS;
    cmd = stripNewline(cmd);
    separateArguments(cmd);

    // Does the user want to exit?
    if (strcmp(Arguments[0].argument, STR_EXIT) == 0) {
        exitShell();
    }
    // Does the user want to run m_agent?
    else if (strcmp(Arguments[0].argument, STR_M_AGENT) == 0) {
        result = doMAgent();
    }
    // Does the user want to run m_cp?
    else if (strcmp(Arguments[0].argument, STR_M_CP) == 0) {
        result = doMCp();
    }
    // Does the user want to run m_run?
    else if (strcmp(Arguments[0].argument, STR_M_RUN) == 0) {
        result = doMRun();
    }
    // No matches? Try to start command
    else {
        result = startProc(Arguments[0].argument);
    }

    return result;
}

//**********************************************************************************************************************
// Mostly done. Check for bugs. Probably need check for >32 agents.
// PJ
/**
 * Handler for m_agent command.\n
 * - `m_agent create <ip> <port>` creates a new entry in the Agents struct\n
 * - `m_agent list` lists all agents\n
 * - `m_agent delete <ip>` deletes any entries with that ip\n
 * @return DSH_EXIT_SUCCESS if successful, DSH_EXIT_ERROR if not
 */
int doMAgent() {
    int index = 0; // Used to save an index after for loop
    int delCount = 0; // Counts how many agents were deleted

    // Check if second argument is present
    if (Arguments[1].argument == NULL) {
        printf(STR_MAGENT_SYNTAX);
        return DSH_EXIT_ERROR;
    }

    // Handle create subcommand
    if (strcmp(Arguments[1].argument, STR_CREATE) == 0) {

        // If third or fourth arg is null, exit method and return error.
        if (Arguments[2].argument == NULL || Arguments[3].argument == NULL) {
            printf(STR_MAGENT_CREATE_SYNTAX);
            return DSH_EXIT_ERROR;
        }

        // Set ip and port of new agent
        for (int i=0; i<32; i++) {
            if (MAgents[i].ip == NULL) {
                MAgents[i].ip = Arguments[2].argument;
                MAgents[i].port = Arguments[3].argument;
                index = i;
                break;
            }
        }
        printf(STR_MAGENT_CREATE_SUCCESS,MAgents[index].ip,MAgents[index].port);
        return DSH_EXIT_SUCCESS;
    }

    // Handle list subcommand
    if (strcmp(Arguments[1].argument, STR_LIST) == 0) {
        for (int i=0; i< 32; i++) {

            //If ip isn't null, print
            if (MAgents[i].ip != NULL) {
                printf(STR_MAGENT_LIST, i, MAgents[i].ip, MAgents[i].port);
                index = i+1;
            }
        }

        // If no agents present, print message so user understands
        if (index == 0) {
            printf(STR_MAGENT_LIST_NONE);
        }
        return DSH_EXIT_SUCCESS;
    }
    // Handle delete subcommand
    if (strcmp(Arguments[1].argument, STR_DELETE) == 0) {

        // If third argument isn't present, exit method
        if (Arguments[2].argument == NULL) {
            printf(STR_MAGENT_DELETE_SYNTAX);
            return DSH_EXIT_ERROR;
        }

        // If ip is found, delete and count
        for (int i=0; i<32; i++) {
            if (MAgents[i].ip != NULL) {
                if (strcmp(Arguments[2].argument, MAgents[i].ip) == 0) {
                    MAgents[i].ip = NULL;
                    MAgents[i].port = NULL;
                    delCount++;
                }
            }
        }
        printf(STR_DELETE_NUMBER, delCount);
        return DSH_EXIT_SUCCESS;
    }
    else {
        // We can print something if we want to specify the error
        return DSH_EXIT_ERROR;
    }
}



//**********************************************************************************************************************
//TODO: Not done
// Lukas
/**
 * Copies a local file using the name provided in dest. The copy
 * is made to the filesystem server and the number of partitions is
 * equal to the number of agents that were created. Send to port 1050-1059 for the file system server
 * @Syntax m_cp local dest
 * @returns DSH_EXIT_ERROR if doMCp has errors and DSH_EXIT_SUCCESS if process successful
 */
int doMCp() {
    // Get the arguments that the user inputted from the arguments struct
    char* local = Arguments[1].argument; // First should be local which is the source file that needs to be copied
    char* dest = Arguments[2].argument; // Next should be dest which is the destination of the local file

    // Get the local file from the provided destination as a binary because we are sending it over the internet
    FILE* file = fopen(dest, "rb");

    // Check if the file exists
    if(file == NULL){
        printf("\nError: could not open file\n");
        return DSH_EXIT_ERROR;
    }

    // Send file to file system server
    for(int i = 0; i < 32; i++) {
        if(MAgents[i].ip != NULL || MAgents[i].port != NULL) {
            sendProgram(local, file, "127.0.0.1", 1050);
        }
    }

    // Return DSH success
    return DSH_EXIT_SUCCESS;
}
//**********************************************************************************************************************

void* waitForResponseFromAgents(void* args) {

    int numAgents = *((int*)&args);

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    char buffer[GINORMOUS_BUFFER];

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server_address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8081);

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
    int numReceived = 0;
    while ((bytes_received = read(client_socket, buffer, GINORMOUS_BUFFER)) >0) {
        if (numReceived == numAgents) {
             break;
        }
        int started = 0;
        int threadToStart;
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received:\n-----\n%s", buffer);


        if (strstr(buffer, "m_run_listener")) {
            char* param = stripNewline(getWordFromString(buffer, 2));
        }
        numReceived++;
    }

    if (bytes_received == -1) {
        perror("Error receiving data");
        exit(EXIT_FAILURE);
    }

    // Close sockets
    close(client_socket);
    close(server_socket);

    return NULL;
}
/**
 * Main program is ran locally with parallel program being sent to each agent that exists
 * @Syntax m_run mainProg parallelProg
 * @returns DSH_EXIT_ERROR if doMCp has errors and DSH_EXIT_SUCCESS if process successful
 */
int doMRun() {
    // create a thread that will monitor for responses
    pthread_t thread;

    // Argument for the thread to know how many responses we are waiting for to continue
    int numAgents = 0;

    // Runs locally
    char *mainProg = Arguments[1].argument;

    // Sent out to each agent
    char *parallelProg = Arguments[2].argument;

    // Open file to send to agents
    FILE* file = fopen(parallelProg, "rb");

    // If file is null, print an error and return DSH_EXIT_ERROR
    if(file == NULL){
        printf("\nERROR: could not open file\n");
        return DSH_EXIT_ERROR;
    }

    // Get count for the number of agents we have so we know how many responses we need
    for(int i =0; i < 32; i++) {
        if ((MAgents[i].ip != NULL) || (MAgents[i].port != NULL)) {
            numAgents = numAgents++;
        }
    }

    // Start thread to wait for responses
    pthread_create(&thread, NULL, (void*) waitForResponseFromAgents, (void*)&numAgents);

    // Distribute the parallel program amongst the existing agents
    for(int i =0; i < 32; i++) {
        if ((MAgents[i].ip != NULL) || (MAgents[i].port != NULL)) {
            // Send the parallel programs
            sendProgram("parallelProgram", file, MAgents[i].ip, atoi(MAgents[i].port));
        }
    }

    // Wait for all responses before continuing
    pthread_join(thread, NULL);

    // Close the file
    fclose(file);

    // return a success
    return DSH_EXIT_SUCCESS;
}

//**********************************************************************************************************************
/**
 * Start external processes. Calls fork syscall then execve syscall.
 *
 * @return 0 if starting process was successful; 1 if not.
 */
int startProc(char* procName) {
    int runProc = 0; // Should we attempt to start the new process?

    // First, see if the provided input points to a file that exists:
    if (access(procName, F_OK) == 0) {
        runProc = 1;
    }
    // No dice? See if it exists in path

    //TODO: Either fix duplicates bug or just don't check them!
    if (runProc == 0) {
        for (int i=0; i<18; i++) {
            char *checkDirectory = malloc(MAX_BUFFER);
            sprintf(checkDirectory, "%s/%s", PathVars[i].DirectoryName, procName);
            if (access(checkDirectory, F_OK) == 0) {
                runProc = 1;
                procName = checkDirectory;
                break;
            }
        }
    }

    // Still nothing? runProc still equals 0, so don't bother trying to run.

    if (runProc) {
        int pid = fork();
        int status = -99;
        // Parent
        if (pid > 0) {
            waitpid(pid, &status, 0);
        }
        // Child
        else {
            execve(procName, NULL, NULL);
            exit(0);
        }
        return 0;
    }
    else {
        // We failed to start the process :(
        return 1;
    }

}
//**********************************************************************************************************************
/**
 * Exits the dsh shell.
 */
void exitShell() {
    exit(0);
}

//**********************************************************************************************************************
/**
 * Removes trailing \\n from char array.
 *
 * @param charArray Character array to strip \\n from
 *
 * @return The stripped char array
 */
char* stripNewline(char* charArr) {
    for (int i=0; i<MAX_BUFFER; i++) {
        if (charArr[i] == '\n') {
            charArr[i] = '\0';
            break;
        }
    }

    return charArr;
}

//**********************************************************************************************************************
/**
 * Set struct for PATH variable
 *
 * @param charArr contains the character array for the arguments we want to set in the struct
 *
 *
 */
char* setStructForArgumentsPATH_VAR(char* charArr) {
    // TODO: We can problem improve this by iterating only for as many times as there is a ":" divider.
    int startingPoint = 0;
    // Name of directory to set in struct
    for (int i=0; i<18; i++) {
        char* dirName = malloc(MAX_BUFFER);

        for (int j=startingPoint; j<MAX_BUFFER; j++) {
            if (charArr[j] == '\0') {
                break;
            }
            if (charArr[j] == ':') {
                // Set starting point
                startingPoint = j+1;
                break;
            }
            dirName[j-startingPoint] = charArr[j];
        }
        // Set struct info
        PathVars[i].DirectoryName = dirName;
    }

}

//**********************************************************************************************************************
/**
 * Separates arguments and sets Arguments struct
 * @param args Character pointer with arguments to separate
 */
void separateArguments(const char* args) {
    char delim = ' ';
    int startingPoint = 0;
    int previousStartPoint = -99;

    // Clear old arguments
    for (int i=0; i<8; i++) {
        Arguments[i].argument = NULL;
    }
    for (int i = 0; i < 8; i++) {
        char *newArg = malloc(MAX_BUFFER);

        for (int j = startingPoint; j < MAX_BUFFER; j++) {

            if (startingPoint == previousStartPoint) {
                i = 7;
                break;
            }

            if (args[j] == '\0') {
                previousStartPoint = startingPoint;
                break;
            }
            if (args[j] == delim) {
                // Set starting point
                startingPoint = j + 1;
                break;
            }
            newArg[j - startingPoint] = args[j];
        }
        // Set struct info
        Arguments[i].argument = newArg;
    }
}

//**********************************************************************************************************************
/**
 * Gets the size of a program
 * @param program Program to get size from
 * @return Size, in bytes of the program
 */
unsigned long getProgramSize(FILE* program) {
    unsigned long programSize;
    fseek(program, 0L, SEEK_END); //Seek to end of program
    programSize = ftell(program); //Save size of program
    rewind(program); // Go back to start of program
    return programSize;
}

//**********************************************************************************************************************
/**
 * Saves a program to a char* buffer.
 * @param program The program to save
 * @param programSize The size of the program
 * @return char* containing the program
 */
char* saveProgramToBuffer(FILE* program, unsigned long programSize) {
    char* programBuffer = malloc(programSize * sizeof(char));
    fread(programBuffer, programSize, 1, program);
    fclose(program); // Close the file
    return programBuffer;
}

//**********************************************************************************************************************
char* readTextToBuffer(FILE* programSrc) {
    char* newText = malloc(GINORMOUS_BUFFER);
    char c;

    do {
        c = fgetc(programSrc);
        if ( c != EOF) {
            sprintf(newText, "%s%c", newText, c);
        }
    }while(c != EOF);

    return newText;
}

//**********************************************************************************************************************
char* getWordFromString(char* word, int pos) {
    int indexCounter = 0;
    int setChars = 0;
    char* newWord = malloc(SMALL_BUFFER);
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

//**********************************************************************************************************************
char* getEverythingAfter(char* word, int pos) {
    int indexCounter = 0;
    int setChars = 0;
    char* newWord = malloc(GINORMOUS_BUFFER);
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

//**********************************************************************************************************************
/**
 * Sends a request to a socket
 * @param requestType The type of request, probably POST
 * @param endpoint The endpoint to hit on the agent
 * @param reqParams Parameters to send in the request.
 * @return DSH_EXIT_SUCCESS if successful, DSH_EXIT_ERROR if not
 */
int sendRequest(char* requestType, char* endpoint, struct sendRequestParam reqParams) {

    const char* progName = reqParams.programName;
    const char* progSrc = reqParams.programSrc;
    const char* n = reqParams.n;
    unsigned int progSize;
    if (progSrc != NULL) {
        progSize = strlen(progSrc);
    }
    else {
        progSize = 0;
    }
    char* param1 = malloc(MID_BUFFER);
    char* param2 = malloc(GINORMOUS_BUFFER);

    if (strcmp(endpoint,"transfer") == 0) {
        strcpy(param1, progName);
        strcpy(param2, progSrc);
    }
    else if (strcmp(endpoint,"run") == 0){
        strcpy(param1, progName);
        strcpy(param2, n);
    }

    char* request = malloc(GINORMOUS_BUFFER); // 4096

    // Based on client example
    struct sockaddr_in servaddr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) <0) {
        perror("No connection");
        return DSH_EXIT_ERROR;
    }
    strcpy(request, endpoint);
    strcat(request, " ");
    strcat(request, param1);
    strcat(request, " ");
    strcat(request, param2);

    long connection = write(sockfd, request, strlen(request));
    if (connection < 0) {perror("Couldn't write to socket"); return DSH_EXIT_ERROR;}

    close(sockfd);
    return DSH_EXIT_SUCCESS;
}

//**********************************************************************************************************************
/**
 * Sends a program to a socket.
 * @param programName Name of the program
 * @param program The program itself
 * @param ip IP address to send to
 * @param port PORT to send to
 * @return DSH_EXIT_SUCCESS if successful, DSH_EXIT_ERROR if not
 */
int sendProgram(const char* programName, FILE* programSrc, const char* ip, const int port) {
    char* programSrcText = readTextToBuffer(programSrc); // Save program to char*
    struct sendRequestParam params = {programName, programSrcText, NULL, ip, port};
    if (sendRequest(HTTP_POST, "transfer", params) != 0) {
        return DSH_EXIT_ERROR;
    }

    return DSH_EXIT_SUCCESS;
}
