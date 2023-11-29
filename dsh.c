#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "constants.h"

// We can use different buffer sizes to save memory
#define BUFFER 32
#define CMD_BUFFER 128
#define MAX_BUFFER 128 // Should equal whatever the biggest buffer is. Used for worst-case scenario stuff.

// Exit codes
enum ExitCode {
    DSH_EXIT_SUCCESS,
    DSH_EXIT_ERROR
};

struct Argument {
    char* argument;
};

/**
 * Initialized global structs for Argument
 */
struct Argument Arguments[32] = {
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

// Structs, which can store params from input
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

// Program function prototypes
int startProc(char* procName);
void exitShell();
int doCommand(char* cmd);
int doMAgent();
int doMCp(char* cmd);
int doMRun(char* cmd);
void initialize();

// Helper function prototypes
char* stripNewline(char* charArr);
//char* separateArguments(char* charArr, enum ArgCase argCase); //DEPRECATED
void separateArguments(const char* args);
char* setStructForArgumentsPATH_VAR(char* charArr);
//char* getFirstArgument(char* charArr, enum ArgCase argCase); //DEPRECATED
void getFirstArgument(char* args);
int setStructForM_AGENT();
char* separateArgumentsFIRST(char* charArr);

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
            printf(STR_FAILSTART,userInput);
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
 * @return 0 if doing command was successful; 1 if failed
 */
int doCommand(char* cmd) {
    // Remove new line from command
    enum ExitCode result = DSH_EXIT_SUCCESS;
    cmd = stripNewline(cmd);
    //TODO: change this to its own seperate arguments
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
        result = doMCp(cmd);
    }
    // Does the user want to run m_run?
    else if (strcmp(Arguments[0].argument, STR_M_RUN) == 0) {
        result = doMRun(cmd);
    }
    // No matches? Try to start command
    else {
        result = startProc(cmd);
    }

    return result;
}

//**********************************************************************************************************************
//TODO: Not done
// PJ
/**
 * @param cmd
 * @return
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
 *
 * @param cmd
 * @return
 */
int doMCp(char* cmd) {
    // Implement me!
    printf("m_cp goes here\n");
    return 0;
}
//**********************************************************************************************************************
//TODO: Not done
// Anothony
/**
 *
 * @param cmd
 * @return
 */
int doMRun(char* cmd) {
    // Implement me!
    printf("m_run goes here\n");
    return 0;
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
 * FIRST: Return the first string in the command.
 *
 * @param charArr The array to separate arguments for
 *
 * @return TODO: What does this return?
 */

//**********************************************************************************************************************
/**
 * Set struct for m_agent
 *
 * @param charArr contains the character array for the arguments we want to set in the struct
 *
 * @return 0 if successful, 1 i f error
 */
int setStructForM_AGENT() {
    /* m_agent has the following:
    * create <ip> <port>
    * list
    * delete <ip>
    */

    int index = 0; // Used to save an index after for loop
    int delCount = 0; // Counts how many agents were deleted

    // Check if second argument is present
    if (Arguments[1].argument == NULL) {
        printf(STR_MAGENT_SYNTAX);
        return DSH_EXIT_ERROR;
    }

    // Handle create subcommand
    if (strcmp(Arguments[1].argument, STR_CREATE) == 0) {
        if (Arguments[2].argument == NULL || Arguments[3].argument == NULL) {
            printf(STR_MAGENT_CREATE_SYNTAX);
            return DSH_EXIT_ERROR;
        }
        for (int i=0; i<31; i++) {
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
            if (MAgents[i].ip != NULL) {
                printf(STR_MAGENT_LIST, i, MAgents[i].ip, MAgents[i].port);
                index = i+1;
            }
        }
        if (index == 0) {
            printf(STR_MAGENT_LIST_NONE);
        }
        return DSH_EXIT_SUCCESS;
    }
    // Handle delete subcommand
    if (strcmp(Arguments[1].argument, STR_DELETE) == 0) {
        if (Arguments[2].argument == NULL) {
            printf(STR_MAGENT_DELETE_SYNTAX);
            return DSH_EXIT_ERROR;
        }
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

    return DSH_EXIT_ERROR;
}

//**********************************************************************************************************************
/**
 * Set struct for PATH variable
 *
 * @param charArr contains the character array for the arguments we want to set in the struct
 *
 * @return TODO: What does this return?
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
    for (int i=0; i<32; i++) {
        Arguments[i].argument = NULL;
    }
    for (int i = 0; i < 32; i++) {
        char *newArg = malloc(MAX_BUFFER);

        for (int j = startingPoint; j < MAX_BUFFER; j++) {

            if (startingPoint == previousStartPoint) {
                i = 31;
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