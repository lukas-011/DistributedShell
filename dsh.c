#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER 32
#define CMD_BUFFER 128

// Program function prototypes
int startProc(char* procName);
void exitShell();
int doCommand(char* cmd);

// Helper function prototypes
char* stripNewline(char* charArr);

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
    //TODO: Let's clean the input rather than compare with \n.
    const char* strExit = "exit";

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
    // FIXME: Absolute paths no work :(
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
    for (int i=0; i<BUFFER; i++) {
        if (charArr[i] == '\n') {
            charArr[i] = '\0';
            break;
        }
    }
    return charArr;
}