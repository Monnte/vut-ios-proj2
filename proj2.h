/**
 * @file proj2.h
 * @author Peter Zdravecký
 * @brief The Faneuil Hall problem head file
 * @version 0.1
 * @date 2020-04-14
 * 
 * @copyright Copyright (c) 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdarg.h>

/**
 * @brief Shared memory structure
 * 
 */
typedef struct
{
    int ProcessCount; /* indexing for every acction process print to file*/
    int NE;           /* Numbers of imigrants in building without certifacate */
    int NC;           /* Number of checked imigrants */
    int NB;           /* All imigrants in building */
    int certLeft;     /* Numbers of certificate that judge need to sign , judge exit condition */
    int judgeconfirm; /* Judge in hall condition */
    int processleft;  /* Exit condtion for parent */
} Memory;

#define fileout "proj2.out"
#define sleepRand(value){if (value != 0) usleep((rand() % ((value + 1) * 1000)));} /* sleep random time by value in miliseconds */

/**
 * @brief Function for judge handling his actions
 * 
 * @param JG max value after judge again enter to building (ms)
 * @param JT max value which simulate time to give certificate (ms)
 */
void judgeProcess(int JG, int JT);

/**
 * @brief Function for imigrants handling their actions
 * 
 * @param id id of imigrant 
 * @param IT max value which simulate time to gain certificate (ms)
 */
void imiProcess(int id, int IT);

/**
 * @brief Initialization of memory and semaphors
 * 
 * @return sucess or error 
 */
int init();

/**
 * @brief Clear seamphores and mapped memory
 * 
 */
void clear();

/**
 * @brief Parse argument to variables and check their validity
 * 
 * @param argc Numbers of arguments
 * @param argv Pointer to arguments
 * @param param Array where arguments will be saved
 * @return Succes or error
 */
int argumentParser(int argc, char *argv[], int *param);

/**
 * @brief Print error message to stderr
 * 
 * @param errmessage message
 */
void printErr(char *errmessage);

/**
 * @brief Print help to stderr
 * 
 * @param prgname Name of program when is executed
 */
void printHelp(char *prgname);