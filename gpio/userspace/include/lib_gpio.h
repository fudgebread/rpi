/***********************************************************************
 * 
 * 
 * GPIO lib function declarations.
 * 
 **********************************************************************/

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
