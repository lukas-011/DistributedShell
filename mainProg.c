#include <bits/types/FILE.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include "binToText.h"
#include <pthread.h>

#define BUFFER_SMALL 32
#define BUFFER_MID 64
#define BUFFER_LARGE 128
#define BUFFER_GINORMOUS 256000


// gather responses from each parallel program and print out the results

// Print out the responses from each agent