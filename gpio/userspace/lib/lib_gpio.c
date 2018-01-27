/***********************************************************************
 * 
 * 
 * GPIO lib function declarations.
 * 
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GPIO_PATH_EXPORT "/sys/class/gpio/export"
#define GPIO_PATH_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_PATH "/sys/class/gpio/gpio%d"
#define GPIO_PATH_DIRECTION "/sys/class/gpio/gpio%d/direction"
#define GPIO_PATH_VALUE "/sys/class/gpio/gpio%d/value"
#define GPIO_PATH_BUFF_MAX 40
#define GPIO_MIN 2
#define GPIO_MAX 27

/* Buffer for gpio path manipulation */
static char g_gpioPathBuff[GPIO_PATH_BUFF_MAX];


/***********************************************************************
 * 
 * Local Functions
 * 
 **********************************************************************/
static int gpioRangeCheck(int gpio)
{
    if (gpio < GPIO_MIN || gpio > GPIO_MAX) {
        printf("Gpio out of range: gpio=%d\n", gpio);
        return 0;
    }
    return 1;
}

static void makeGpioPath(int gpio, char *path)
{
    memset(g_gpioPathBuff, 0, GPIO_PATH_BUFF_MAX);
    sprintf(g_gpioPathBuff, path, gpio);
}

static int gpioOpen(int gpio)
{
    makeGpioPath(gpio, GPIO_PATH);
    if (access(g_gpioPathBuff, F_OK) != 0) {
        printf("Gpio not exported: gpio=%d\n", gpio);
        return 0;
    }
    
    return 1;
}
/***********************************************************************
 * 
 * API
 * 
 **********************************************************************/

int libGpioOpen(int gpio)
{
    char tmpBuff[4];
    int fd;
    
    if (!gpioRangeCheck(gpio))
        return -1;        
    
    // check gpio already open
    makeGpioPath(gpio, GPIO_PATH);
    if (access(g_gpioPathBuff, F_OK) == 0) {
        printf("Gpio already exported: gpio=%d\n", gpio);
        return -2;
    }
    
    // echo [gpio] > export
    fd = open(GPIO_PATH_EXPORT, O_WRONLY);
    if (fd <= 0) {
        printf("Failed to export gpio: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    sprintf(tmpBuff, "%d", gpio);
    write(fd, tmpBuff, 4);
    close(fd);
    
    return 0;
}

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
int libGpioClose(int gpio)
{
    char tmpBuff[4];
    int fd;
    
    if (!gpioRangeCheck(gpio))
        return -1;        
    
    if (!gpioOpen(gpio))
        return -2;
    
    // echo [gpio] > unexport
    fd = open(GPIO_PATH_UNEXPORT, O_WRONLY);
    if (fd <= 0) {
        printf("Failed to unexport gpio: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    sprintf(tmpBuff, "%d", gpio);
    write(fd, tmpBuff, 4);
    close(fd);
    
    return 0;
}

int libGpioDirection(int gpio, int direction)
{
    int fd;
    if (!gpioRangeCheck(gpio))
        return -1;    
        
    if (!gpioOpen(gpio))
        return -2;
        
    makeGpioPath(gpio, GPIO_PATH_DIRECTION);
    
    fd = open(g_gpioPathBuff, O_WRONLY);
    if (fd <= 0) {
        printf("Failed to set gpio direction: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    if (direction)
        write(fd, "in", 2);
    else
        write(fd, "out", 3);
        
    close(fd);
    
    return 0;
}

int libGpioBitRead(int gpio, int *value)
{
    int fd;
    char val[3];
    
    if (!gpioRangeCheck(gpio))
        return -1;    
        
    if (!gpioOpen(gpio))
        return -2;
        
    makeGpioPath(gpio, GPIO_PATH_VALUE);
    
    fd = open(g_gpioPathBuff, O_RDONLY);
    if (fd <= 0) {
        printf("Failed to open gpio value: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    if (read(fd, val, 3) < 0) {
        printf("Failed to read gpio value: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
 
    close(fd);
 
    *value = atoi(val);
    return 0;
}

int libGpioBitWrite(int gpio, int value)
{
    int fd;
    char val[3];
    
    if (!gpioRangeCheck(gpio))
        return -1;    
        
    if (!gpioOpen(gpio))
        return -2;
        
    makeGpioPath(gpio, GPIO_PATH_VALUE);
    
    fd = open(g_gpioPathBuff, O_WRONLY);
    if (fd <= 0) {
        printf("Failed to open gpio value: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    sprintf(val, "%d", value);
    if (write(fd, val, 3) < 0) {
        printf("Failed to wrte gpio value: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
 
    close(fd);
 
    return 0;
}
