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
    M_RUN
};

// Program function prototypes
int startProc(char* procName);
void exitShell();
int doCommand(char* cmd);
int doMAgent(char* cmd);

// Helper function prototypes
char* stripNewline(char* charArr);
char* separateArguments(char* charArr, enum ArgCase argCase);





/**
 * Program execution starts here
 *
 * @return 9 If we somehow break from the while loop.
 */
int main() {
    while(1) {
        char userInput[CMD_BUFFER];
        printf(">");
        fgets(userInput, CMD_BUFFER, stdin);
        doCommand(userInput);
    }
}

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

    if (strcmp(baseCmd, STR_EXIT) == 0) {
        exitShell();
    }
    else if (strcmp(baseCmd, STR_M_AGENT) == 0) {
        doMAgent(cmd);
    }
    // No matches? Try to start command
    else {
        startProc(cmd);
    }
    return 0;
}

int doMAgent(char* cmd) {
    // Implement me!
    printf("m_agent goes here\n");
}

/**
 * Start external processes. Calls fork syscall then execve syscall.
 *
 * @return 0 if starting process was successful; 1 if not.
 */
int startProc(char* procName) {
    // TODO: Check if file exists
    // TODO: Implement running commands from PATH variable.
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
 *      - FIRST: Return the first command.
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
        default: {
            // Implement me! Should save arg for execve.
            return charArr;
        }
    }
}