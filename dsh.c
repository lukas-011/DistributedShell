#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
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

    // == Uncomment below to work on sending test ==
    sendProgram("lampoil",
                fopen("/home/pj/TestSrc/lampoil.c", "r"),
                "127.0.0.1",
                8080);
    return 9;
    // == End test stuff ==
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

    // Copy the contents of the file to a variable
    // allocate memory for the string
    fseek(file, 0L, SEEK_END);
    unsigned long lengthOfFile = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to store the contents of our file to a string
    char* contents = (char*)malloc(lengthOfFile + 1);

    // Check if the contents are null
    if(contents == NULL){
        fclose(file);
        printf("\nERROR: No contents in the file");
        return DSH_EXIT_ERROR;
    }

    // Copy the contents of the file to the string and then add a '\0' to signify the end of the file
    fread(contents, 1, lengthOfFile, file);
    contents[lengthOfFile] = '\0';

    // close the file since we are done with it
    fclose(file);

    // Send the contents over to the filesystem server depending on how many agents there are
    for(int i = 0; i < 32; i++) {
        // check if there is an agent by checking for exiting ip and port for each agent
        if(MAgents[i].ip != NULL || MAgents[i].port != NULL) {
            // TODO: what does program name have to be
            struct sendRequestParam copyRequest = {local, contents, n, MAgents[i].ip, MAgents[i].port};
        }
    }

    // Return DSH success
    return DSH_EXIT_SUCCESS;
}
//**********************************************************************************************************************
//TODO: Not done
// Anthony
/**
 * Main program is ran locally with parallel program being sent to each agent that exists
 * @Syntax m_run mainProg parallelProg
 * @returns DSH_EXIT_ERROR if doMCp has errors and DSH_EXIT_SUCCESS if process successful
 */
int doMRun() {
    //runs locally
    char *mainProg = Arguments[1].argument;

    //sent out to each agent.
    char *parallelProg = Arguments[2].argument;

    FILE* file = fopen(parallelProg, "rb");

    // Check if the file exists
    if(file == NULL){
        printf("\nError: could not open file\n");
        return DSH_EXIT_ERROR;
    }

    // Copy the contents of the file to a variable
    // allocate memory for the string
    fseek(file, 0L, SEEK_END);
    unsigned long lengthOfFile = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to store the contents of our file to a string
    char* contents = (char*)malloc(lengthOfFile + 1);

    // Check if the contents are null
    if(contents == NULL){
        fclose(file);
        printf("\nERROR: No contents in the file");
        return DSH_EXIT_ERROR;
    }

    // Copy the contents of the file to the string and then add a '\0' to signify the end of the file
    fread(contents, 1, lengthOfFile, file);
    contents[lengthOfFile] = '\0';

    // Start the main process locally
    startProc(mainProg);

    // Distribute the parallel program amongst the existing agents
    for(int i =0; i < 32; i++) {
        if ((MAgents[i].ip != NULL) || (MAgents[i].port != NULL)) {
            // Set struct to send
            struct sendRequestParam parallelStruct = {parallelProg, contents, n, MAgents[i].ip, MAgents[i].port};
            // Send the parallel programs

        }
    }
    return DSH_EXIT_SUCCESS;
}

//**********************************************************************************************************************
/**
 * Start external processes. Calls fork syscall then execve syscall.
 *
 * @return 0 if starting process was successful; 1 if not.
 */
int startProc(char* procName) {
    // TODO: Check if file exists
    // TODO: Implement running commands from PATH variable.
    // TODO: Add support for arguments
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

    char* request = malloc((progSize*2)+GINORMOUS_BUFFER); // Enough room for program ascii plus 4096

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

    // Allocate request lines and fill out the lines
    char* reqLineHeader = malloc(MID_BUFFER);
    char* reqLineHost = malloc(MID_BUFFER);
    char* reqLineContentType = malloc(MID_BUFFER);
    char* reqLineCount = malloc(MID_BUFFER);
    char* reqLineParams = malloc(MID_BUFFER);

    sprintf(reqLineHeader, "%s /%s HTTP/1.1\r\n", requestType, endpoint);
    sprintf(reqLineHost, "HOST: %s\r\n", getenv(STR_HOST));
    sprintf(reqLineContentType, "Content-Type: application/x-www-form-urlencoded\r\n");
    if (reqParams.programName != NULL) {
        sprintf(reqLineParams, "programName=%s",progName);
    }
    if (reqParams.programSrc != NULL) {
        sprintf(reqLineParams, "%s&programSrc=%s",reqLineParams, progSrc);
    }
    if (reqParams.n != NULL) {
        sprintf(reqLineParams, "%s&argument=%s", reqLineParams, n);
    }
    sprintf(reqLineCount, "Content-Length: %lu\r\n\r\n", strlen(reqLineParams));


    // Build the request
    strcpy(request, reqLineHeader);
    strcat(request, reqLineHost);
    strcat(request, reqLineContentType);
    strcat(request, reqLineCount);
    strcat(request, reqLineParams);

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