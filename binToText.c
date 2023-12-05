#include "binToText.h"
#include <stdio.h>
#include "constants.h"
#include <malloc.h>
/**
 * Encodes a binary into text. Make sure to free
 * @param binary - A char* containing hexadecimal
 * @param length - The size of the binary
 * @return An ASCII representation of the hexadecimal
 */
char* encodeBinary(const char* binary, unsigned long length) {

    char* newText = malloc(2*(length * (sizeof(char)))); // The buffer is the size of the program * 2 characters each
    for (unsigned long i = 0; i < length; i++) {
        for (int j=0; j<256;j++) {
            if (binary[i] == HEXVALS[j]) {
                sprintf(newText, "%s%02X", newText, j);
                break;
            }
            else {
                if (j == 255) {
                    printf("OH NO\n");
                }

            }
        }
    }
    //newText[length] = '\0';

    return newText;
}

/**
 * Decodes hexadecimal text into true hexadecimal
 * @param encodedBinary - An ASCII representation of hexadecimal
 * @param length - The length of the binary
 * @return A buffer containing the hexadecimal for a binary program
 */
char* decodeBinary(const char* encodedBinary, unsigned long length) {
    char* newBinary = malloc(length * sizeof(char));
    unsigned long indexToSetBit = 0;

    for (unsigned long i=0; i<length*2;i+=2) {
        char encodedTest[2] = {encodedBinary[i], encodedBinary[i+1]};
        for (int j=0; j<256;j++) {
            char* compareChar = malloc(2);
            sprintf(compareChar, "%02X", j);
            if ((compareChar[0] == encodedTest[0]) && (compareChar[1] == encodedTest[1])) {
                newBinary[indexToSetBit] = HEXVALS[j];
                break;
            }
            free(compareChar);
        }
        indexToSetBit++;
    }
    return newBinary;
}