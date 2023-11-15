#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER 32


int startProc();
int exitShell();
int doCommand(const char* cmd);

/*
 * Program execution starts here
 */
int main() {
    while(1) {
        char userInput[BUFFER];
        printf(">");
        fgets(userInput, BUFFER, stdin);
        doCommand(userInput);
    }
}

/*
 * Determine which method to call based on user input
 */
int doCommand(const char* cmd) {
    //TODO: Let's clean the input rather than compare with \n.
    const char* strExit = "exit\n";
    // Compare with if internal command (m_...)
    // See if text is "exit"
    // Else, call startProc and run process

    if (strcmp(cmd, strExit) == 0) {
        exitShell();
    }
    return 0;
}

/*
 * Start external processes. Calls fork syscall then execve syscall.
 */
int startProc() {
    return 0;
}

int exitShell() {
    exit(0);
}