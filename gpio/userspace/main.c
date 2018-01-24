/***********************************************************************
 * 
 * 
 * GPIO examples
 * 
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "include/lib_gpio.h"

int main(int argc, char *argv[])
{
	printf("Starting gpio examples...\n");
	
	printf("Opening GPIO 23 (pin 16)\n");
	
	if (libGpioOpen(23) != 0)
		return -1;
		
	if (libGpioDirection(23, 0) != 0)
		return -1;
		
	return 0;
}
