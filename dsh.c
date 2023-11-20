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

// Helper function prototypes
char* stripNewline(char* charArr);
char* separateArguments(char* charArr, enum ArgCase internalCmd);





/**
 * Program execution starts here
 *
 * @return 9 If we somehow break from the while loop.
 */
int main() {

    char* big = separateArguments("big john",FIRST);
    printf("Look out for the: %s\n",big);

    while(1) {
        char userInput[CMD_BUFFER];
        printf(">");
        fgets(userInput, CMD_BUFFER, stdin);
        doCommand(userInput);
    }
    return 9;
}

/**
 * Determine which method to call based on user input
 *
 * @param cmd Command to run
 *
 * @return 0 if doing command was successful; 1 if failed
 */
int doCommand(char* cmd) {

    // Strings to compare against
    const char* strExit = "exit";
    const char* strM_agent = "m_agent";
    const char* strM_cp = "m_cp";
    const char* strM_run = "m_run";

    cmd = stripNewline(cmd);
    // Compare with if internal command (m_...)
    // See if text is "exit"
    // Else, call startProc and run process

    if (strcmp(cmd, strExit) == 0) {
        exitShell();
    }
    // No matches? Try to start command
    else {
        startProc(cmd);
    }
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
    // FIXME: Past 4 levels programs won't start in absolute path?
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

char* separateArguments(char* charArr, enum ArgCase internalCmd) {
    switch (internalCmd) {
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