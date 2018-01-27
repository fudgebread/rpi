/***********************************************************************
 * 
 * 
 * GPIO examples
 * 
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "include/lib_gpio.h"

#define CMD_STATUS "status"
#define CMD_OPEN "open"
#define CMD_CLOSE "close"
#define CMD_DIR "dir"
#define CMD_READ "read"
#define CMD_WRITE "write"
#define CMD_EXIT "quit"

/* Idea here is we start/stop polling threads using a simple boolean
 * enable guarded by one mutex
 */
static pthread_t pollThreads[GPIO_TOTAL] = {0};
static int pollEnable[GPIO_TOTAL] = {0};
static pthread_mutex_t pollMutexes[GPIO_TOTAL];

static void printHelp()
{
    printf("       status [open|close|dir|read|write|poll] <gpio> [in|out] [value]\n");
}

static void printStatus()
{
    int i;
    gpioStatus_t info;
    
    printf("\n");
    printf(" GPIO | Open | Dir | Value\n"); 
    printf(" -----+------+-----+------\n"); 
    for (i=GPIO_MIN; i<=GPIO_MAX; i++) {
        if (libGpioStatus(i, &info) == 0) {
            if (info.open) {
                printf(" %-4d | %-4s | %-3s | %-4d\n",
                        i, info.open ? "yes" : "no",
                        info.direction ? "in" : "out",
                        info.value); 
            }
            else {
                printf(" %-4d | %-4s | %-3s | %-4s\n",
                        i, "no", "NA", "NA"); 
            }
        }
        else {
            printf(" %-3d| ???  |    ???    |   ???  \n", i); 
        }
    }
    printf("\n");
}

static void* pollGpio(void *args)
{
    int i, gpio = *((int*)args);
    
    // LOCK; tmpEnable = mainEnable; UNLOCK
    
    // while tmpEnable
    // get reading
    // LOCK; get new tmpEnable; UNLOCK
    
    // Add locking to lib now
    return 0;
}

static void* getUserInput(void *args)
{
    int gpio, value, i, run=1;
    char input[80], command[10], dir[5];
    
    while (run) {
        printf("GPIO > ");
        fgets(input, 80, stdin);
        
        if (sscanf(input, "%s %d %s\n", command, &gpio, dir) < 1) {
            printf("GPIO > invalid command.  Try help\nGPIO > ");
        }
        
        if (strcmp(command, CMD_STATUS) == 0) {
            printStatus();
        }
        else if (strcmp(command, CMD_OPEN) == 0) {
            libGpioOpen(gpio);
        }
        else if (strcmp(command, CMD_CLOSE) == 0) {
            libGpioClose(gpio);
        }
        else if (strcmp(command, CMD_DIR) == 0) {
            if (strcmp(dir, "in") == 0) {
                libGpioDirection(gpio, 1);
            }
            else if (strcmp(dir, "out") == 0) {
                libGpioDirection(gpio, 0);
            }
            else {
                printHelp();
            }
        }
        else if (strcmp(command, CMD_READ) == 0) {
            if (libGpioBitRead(gpio, &value) == 0) {
                printf("GPIO > value = %d\n", value);
            }
            else {
                printf("GPIO > error: read failed\n");
            }
        }
        else if (strcmp(command, CMD_WRITE) == 0) {
            libGpioBitWrite(gpio, atoi(dir));
        }
        else if (strcmp(command, CMD_EXIT) == 0) {
            run = 0;
        }
        else {
            printHelp();
        }
    }
    
    printf("GPIO > fin!\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int status, startConsole = 0;
    pthread_t userThread;
    
    while ((status = getopt(argc, argv, "c")) != -1) {
        switch (status) {
            case 'c': 
            startConsole = 1; 
            break;
        default:
            fprintf(stderr, "Usage: %s [-c]\n", argv[0]);
            return -1;
        }
    }
    
    if (startConsole) {
        printf("Starting gpio console\n");
        
        status = pthread_create(&userThread, NULL, getUserInput, NULL);
        if (status != 0) {
            fprintf(stderr, "Could not create user input thread (%d)\n", errno);
            return -1;
        }
        
        status = pthread_join(userThread, NULL);
        if (status != 0) {
            fprintf(stderr, "Could not join user input thread (%d)\n", errno);
            return -1;
        }
    }
        
    return 0;
}
