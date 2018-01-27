/***********************************************************************
 * 
 * GPIO lib function declarations.
 * 
 **********************************************************************/
 
#define GPIO_MIN 2
#define GPIO_MAX 26
#define GPIO_TOTAL (GPIO_MAX-GPIO_MIN+1)

/**
 * GPIO information encapsulation.
 * TODO: this could use enums.
 **/
typedef struct gpioStatus_s {
	int open;      /* gpio open? 1==open */
	int gpio;      /* gpio number */
	int direction; /* directin 1==in, 0==out */
	int value; 	   /* gpio value */
} gpioStatus_t;

/**
 * Open a gpio port for reading/writing.
 * 
 * @param gpio the gpio pin
 * 
 * @return 0 success
 * @return -1 invalid gpio
 * @return -2 gpio already open
 * @return -3 other error occurred
 **/
int libGpioOpen(int gpio);

/**
 * Close a gpio port.
 * 
 * @param gpio the gpio pin
 * 
 * @return 0 success
 * @return -1 invalid gpio
 * @return -2 gpio not open
 * @return -3 other error occurred
 **/
int libGpioClose(int gpio);

/**
 * Set gpio direction.
 * 
 * @param gpio the gpio pin
 * @param direction gpio direction 1==in, 0==out
 * 
 * @return 0 success
 * @return -1 invalid gpio
 * @return -2 gpio not open
 * @return -3 other error occurred
 **/
int libGpioDirection(int gpio, int direction);

/**
 * Read a gpio pin. 
 * 
 * @param gpio the gpio pin
 * @param value (out) the value read
 * 
 * @return 0 success
 * @return -1 invalid gpio
 * @return -2 gpio not open
 * @return -3 other error occurred
 **/
int libGpioBitRead(int gpio, int *value);

/**
 * Write to a gpio pin.
 * 
 * @param gpio the gpio pin
 * @param value the value write
 * 
 * @return 0 success
 * @return -1 invalid gpio
 * @return -2 gpio not open
 * @return -3 other error occurred
 **/
int libGpioBitWrite(int gpio, int value);

/**
 * Get the status for a gpio.
 * 
 * @param gpio the gpio pin
 * @param info (out) the gpio's status (direction, value, etc.)
 * 
 * @return 0 success
 * @return -1 invalid gpio
 **/
int libGpioStatus(int gpio, gpioStatus_t *info);
