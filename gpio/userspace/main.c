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
#include <syslog.h>

#include "include/lib_gpio.h"

#define CMD_STATUS "status"
#define CMD_OPEN "open"
#define CMD_CLOSE "close"
#define CMD_DIR "dir"
#define CMD_READ "read"
#define CMD_WRITE "write"
#define CMD_EXIT "quit"


static void printHelp()
{
	printf(" Help\n");
	printf(" =====\n");
	printf(" status                - display status of all gpio \n");
	printf(" open <gpio>           - open <gpio>\n");
	printf(" close <gpio>          - close gpio <gpio>\n");
	printf(" dir <gpio> [in/out]   - set direction for gpio\n");
	printf(" read <gpio>           - read gpio <gpio>\n");
	printf(" write <gpio> <value>  - write value to <gpio>\n");
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
                        info.direction ? "out" : "in",
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
            libGpioMemMapSelect(gpio, gpioSel_all, 0); // clear all
        }
        else if (strcmp(command, CMD_CLOSE) == 0) {
			libGpioMemMapSelect(gpio, gpioSel_all, 0); // clear all
        }
        else if (strcmp(command, CMD_DIR) == 0) {
            if (strcmp(dir, "in") == 0) {
                libGpioMemMapSelect(gpio, gpioSel_input, 1); // set input
            }
            else if (strcmp(dir, "out") == 0) {
                libGpioMemMapSelect(gpio, gpioSel_output, 1); // set out
            }
            else {
                printHelp();
            }
        }
        else if (strcmp(command, CMD_READ) == 0) {
            if (libGpioMemMapRead(gpio, &value) == 0) {
                printf("GPIO > value = %d\n", value);
            }
            else {
                printf("GPIO > error: read failed\n");
            }
        }
        else if (strcmp(command, CMD_WRITE) == 0) {
			if (atoi(dir) >= 1) {
				libGpioMemMapSet(gpio);
			}
			else {
				libGpioMemMapClear(gpio);
			}
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
    
    /* Open logger */
    openlog(argv[0], 0, LOG_USER);
    
    /* Check user args */
    while ((status = getopt(argc, argv, "ct")) != -1) {
        switch (status) {
            case 'c': 
				startConsole = 1; 
				break;
            case 't':
				startConsole = 0;
				break;
        default:
            fprintf(stderr, "Usage: %s [-c]\n", argv[0]);
            return -1;
        }
    }
    
    if (!startConsole) {
		int i, gpio = 11;

		if (libGpioMemMapInit() == -1)
			return -1;
		
		libGpioMemMapSelect(11, gpioSel_all, 0); // clear all
		libGpioMemMapSelect(11, gpioSel_output, 1); // set out
		libGpioMemMapSet(11); // write 1
		sleep(5);
		libGpioMemMapClear(11); // clear 1
		return 0;
	}
        
    /* Kick off the console if needs be. This is the only arg at the moment
     * so we just block until the console exits
     */
    if (startConsole) {

		if (libGpioMemMapInit() == -1)
			return -1;
			
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
    
    closelog();
        
    return 0;
}
