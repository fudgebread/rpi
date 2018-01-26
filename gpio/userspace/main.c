/***********************************************************************
 * 
 * 
 * GPIO examples
 * 
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/lib_gpio.h"

int main(int argc, char *argv[])
{
	int i, gpio=26;
	printf("Starting gpio examples...\n");
	
	printf("Opening GPIO %d\n", gpio);
	
	if (libGpioOpen(gpio) != 0)
		return -1;
		
	if (libGpioDirection(gpio, 0) != 0)
		return -1;
		
	for (i=0; i<10; i++) {
		printf("Write 1 to gpio %d\n", gpio);
		if (libGpioBitWrite(gpio, 1) != 0) {
			return -1;
		}
		sleep(1);
		
		printf("Write 0 to gpio %d\n", gpio);
		if (libGpioBitWrite(gpio, 0) != 0) {
			return -1;
		}
		sleep(1);
	}
	
	if (libGpioClose(gpio) != 0)
		return -1;
		
	return 0;
}
