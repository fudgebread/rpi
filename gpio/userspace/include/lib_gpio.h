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


typedef enum gpioSel {
    gpioSel_input = 0,
    gpioSel_output = 1,
    gpioSel_alt0 = 4,
    gpioSel_alt1 = 5,
    gpioSel_alt2 = 6,
    gpioSel_alt3 = 7,
    gpioSel_alt4 = 3,
    gpioSel_alt5 = 2,
    gpioSel_all = gpioSel_alt3
} gpioSel_t;

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
int libGpioSysFsOpen(int gpio);

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
int libGpioSysFsClose(int gpio);

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
int libGpioSysFsDirection(int gpio, int direction);

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
int libGpioSysFsBitRead(int gpio, int *value);

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
int libGpioSysFsBitWrite(int gpio, int value);

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

/**
 * Get the status for a gpio.
 * 
 * @param gpio the gpio pin
 * @param info (out) the gpio's status (direction, value, etc.)
 * 
 * @return 1 gpio is in range
 * @return 0 gpio is out-of-range
 **/
int libGpioRangeCheck(int gpio);

/**
 * Clear a gpio pin.
 * 
 * @param gpio the gpio pin
 * 
 * @return 0 if the set was successful
 * @return -1 if the gpio was out of range
 **/
int libGpioMemMapClear(int gpio);

/**
 * Write a value to a gpio pin.
 * 
 * @param gpio the gpio pin
 * 
 * @return 0 if the set was successful
 * @return -1 if the gpio was out of range
 **/
int libGpioMemMapSet(int gpio);

/**
 * Set the function of a gpio pin. Note, a full GPIO ping reset is
 * performed by clearing gpioSel_alt3.
 * 
 * @param gpio the gpio pin to set
 * @param sel the function to set
 * @param set whether or not to set (1) or clear (0) this function
 * 
 * @return 0 if the set was successful
 * @return -1 if the gpio was out of range
 **/
int libGpioMemMapSelect(int gpio, gpioSel_t sel, int set);

/**
 * Initialise memory-mapped IO. This only maps physical memory to
 * virtual memory: no GPIO initialisation is performed!
 * 
 * @return 0 if initialisation was successful
 * @return -1 if there was an error
 **/
int libGpioMemMapInit();

/**
 * Read the high or low value fro ma gpio pin.
 * 
 * @param gpio the gpio pin to read
 * @param value (out) the value read
 * 
 * @return 0 if the read was successful
 * @return -1 if the gpio was out of range
 **/
int libGpioMemMapRead(int gpio, int *value);
