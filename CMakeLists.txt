cmake_minimum_required(VERSION 3.26)
project(DistributedShell C)

set(CMAKE_C_STANDARD 11)

add_executable(DistributedShell dsh.c
        binToText.c
        binToText.h)
#add_executable(DistributedShell agent.c
#        binToText.c
#        binToText.h)