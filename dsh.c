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

// Arguments, used in separateArguments.
enum ArgCase {
    FIRST,
    M_AGENT,
    M_CP,
    M_RUN,
    PATH_VAR
};

// Structs, which can store params from input
// lol

struct PathVar {
    char* DirectoryName;
};

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

struct MAgentParam {
    char* param;
    char* ip;
    char* port;
};

struct MAgentParam MAgentParams[1] = { {} };

// Program function prototypes
int startProc(char* procName);
void exitShell();
int doCommand(char* cmd);
int doMAgent(char* cmd);
int doMCp(char* cmd);
int doMRun(char* cmd);
int initialize();

// Helper function prototypes
char* stripNewline(char* charArr);
char* separateArguments(char* charArr, enum ArgCase argCase);

/**
 * Program execution starts here
 *
 * @return 9 If we somehow break from the while loop.
 */
int main() {
    initialize();
    printf("%s",STR_GREETING);

    while(1) {
        char userInput[CMD_BUFFER];
        printf(">");
        fgets(userInput, CMD_BUFFER, stdin);
        doCommand(userInput);
    }
}

/**
 * Initialize defaults for the program
 * @return 0 on exit
 */
int initialize() {
    // Set PATH struct
    separateArguments(getenv(STR_PATH), PATH_VAR);

    return 0;
}

// kacki likes grapes

/**
 * Determine which method to call based on user input
 *
 * @param cmd Command to run
 *
 * @return 0 if doing command was successful; 1 if failed
 */
int doCommand(char* cmd) {
    char* baseCmd;

    // Remove new line from command
    cmd = stripNewline(cmd);
    baseCmd = separateArguments(cmd, FIRST);

    // Does the user want to exit?
    if (strcmp(baseCmd, STR_EXIT) == 0) {
        exitShell();
    }
    // Does the user want to run m_agent?
    else if (strcmp(baseCmd, STR_M_AGENT) == 0) {
        doMAgent(cmd);
    }
    // Does the user want to run m_cp?
    else if (strcmp(baseCmd, STR_M_CP) == 0) {
        doMCp(cmd);
    }
    // Does the user want to run m_run?
    else if (strcmp(baseCmd, STR_M_RUN) == 0) {
        doMRun(cmd);
    }
    // No matches? Try to start command
    else {
        startProc(cmd);
    }

    free(baseCmd);
    return 0;
}

int doMAgent(char* cmd) {
    // Implement me!
    printf("m_agent goes here\n");
    // Do separate again
    return 0;
}

int doMCp(char* cmd) {
    // Implement me!
    printf("m_cp goes here\n");
    return 0;
}

int doMRun(char* cmd) {
    // Implement me!
    printf("m_run goes here\n");
    return 0;
}

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

/**
 * Exits the dsh shell.
 */
void exitShell() {
    exit(0);
}

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

/**
 * Separate arguments.
 * @param charArr The array to pick apart
 * @param argCase The case which determines where arguments go:
 *      \n- FIRST: Return the first string in the command.
 *      \n- M_AGENT: Set struct for m_agent
 *      \n- M_CP: Set struct for m_cp
 *      \n- M_RUN: Set struct for m_run
 *      \n- PATH_VAR: Set struct for PATH variable
 * @return
 */
char* separateArguments(char* charArr, enum ArgCase argCase) {
    switch (argCase) {
        case (FIRST): {
            // The goal: Get returnString to equal the first "string" in charArr
            char* returnString = malloc(sizeof(charArr));
            for (int i=0; i<MAX_BUFFER; i++) {
                // If we reach end of string, break to avoid outta bounds
                if (charArr[i] == '\0') {
                    break;
                }
                // If we find a space, stop the count!
                if (charArr[i] == ' ') {
                    break;
                }
                returnString[i] = charArr[i];
            }
            return returnString;

        }
        case (M_AGENT): {
            /* m_agent has the following:
             * create <ip> <port>
             * list
             * delete <ip>
             */
            for (int i=0;i<MAX_BUFFER;i++) {

            }
            break;
        }
        case (PATH_VAR): {
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

                //FIXME: Not freed! Is this okay?
            }

        }

    }
    // Pro Tip: Seg fault? Might have not returned in switch!
    return NULL;
}