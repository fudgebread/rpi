/***********************************************************************
 * 
 * 
 * GPIO lib function declarations.
 * 
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#include "../include/lib_gpio.h"

#define GPIO_PATH_EXPORT "/sys/class/gpio/export"
#define GPIO_PATH_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_PATH "/sys/class/gpio/gpio%d"
#define GPIO_PATH_DIRECTION "/sys/class/gpio/gpio%d/direction"
#define GPIO_PATH_VALUE "/sys/class/gpio/gpio%d/value"
#define GPIO_PATH_BUFF_MAX 40
#define USER_TO_GPIO(a) (a-GPIO_MIN)

/* Buffer for gpio path manipulation */
static char gpioPathBuff[GPIO_PATH_BUFF_MAX];

/* GPIO info array */
static gpioStatus_t gpioInfo[GPIO_TOTAL];

/* Whether or not we have initialised the info array */
static int initialised = 0;

static pthread_mutex_t gpioMutexes[GPIO_TOTAL];

/***********************************************************************
 * 
 * Local Functions
 * 
 **********************************************************************/
int libGpioRangeCheck(int gpio)
{
    if (gpio < GPIO_MIN || gpio > GPIO_MAX) {
        syslog(LOG_ERR, "Gpio out of range: gpio=%d\n", gpio);
        return 0;
    }
    return 1;
}

static void makeGpioPath(int gpio, char *path)
{
    memset(gpioPathBuff, 0, GPIO_PATH_BUFF_MAX);
    sprintf(gpioPathBuff, path, gpio);
}

static int gpioOpen(int gpio)
{
    makeGpioPath(gpio, GPIO_PATH);
    if (access(gpioPathBuff, F_OK) != 0) {
        syslog(LOG_ERR, "Gpio not exported: gpio=%d\n", gpio);
        return 0;
    }
    
    return 1;
}

/***********************************************************************
 * 
 * API
 * 
 **********************************************************************/
void libGpioInit(int gpio)
{
    int i, j;
    
    if (initialised)
        return;
        
    for (i=0, j=GPIO_MIN; i<GPIO_TOTAL; i++, j++) {
        memset(&gpioInfo[i].gpio, 0, sizeof(gpioStatus_t));
        gpioInfo[i].gpio = j;
        gpioInfo[i].open = 0;
        
        pthread_mutex_init(&gpioMutexes[i], NULL);
    }
    initialised = 1;
}

int libGpioOpen(int gpio)
{
    char tmpBuff[4];
    int fd;
    
    if (!libGpioRangeCheck(gpio))
        return -1;        
    
    if (gpioInfo[USER_TO_GPIO(gpio)].open) {
        syslog(LOG_ERR, "GPIO already open: gpio=%d, errno=%d\n", gpio, errno);
        return 0;
    }
    
    // echo [gpio] > export
    fd = open(GPIO_PATH_EXPORT, O_WRONLY);
    if (fd <= 0) {
        syslog(LOG_ERR, "Failed to export gpio: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    sprintf(tmpBuff, "%d", gpio);
    write(fd, tmpBuff, 4);
    close(fd);
    
    gpioInfo[USER_TO_GPIO(gpio)].open = 1;
    
    return 0;
}

int libGpioClose(int gpio)
{
    char tmpBuff[4];
    int fd;
    
    if (!libGpioRangeCheck(gpio))
        return -1;        
    
    // if it's not open then it's already closed
    if (!gpioInfo[USER_TO_GPIO(gpio)].open) {
        syslog(LOG_ERR, "GPIO already closed: gpio=%d, errno=%d\n", gpio, errno);
        return 0;
    }
    
    // echo [gpio] > unexport
    fd = open(GPIO_PATH_UNEXPORT, O_WRONLY);
    if (fd <= 0) {
        syslog(LOG_ERR, "Failed to unexport gpio: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    sprintf(tmpBuff, "%d", gpio);
    write(fd, tmpBuff, 4);
    close(fd);
    
    gpioInfo[USER_TO_GPIO(gpio)].open = 0;
    return 0;
}

int libGpioDirection(int gpio, int direction)
{
    int fd;
    if (!libGpioRangeCheck(gpio))
        return -1;    
        
    if (!gpioInfo[USER_TO_GPIO(gpio)].open) {
        syslog(LOG_ERR, "GPIO not open: gpio=%d, errno=%d\n", gpio, errno);
        return -2;
    }
        
    makeGpioPath(gpio, GPIO_PATH_DIRECTION);
    
    fd = open(gpioPathBuff, O_WRONLY);
    if (fd <= 0) {
        syslog(LOG_ERR, "Failed to set gpio direction: gpio=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    if (direction)
        write(fd, "in", 2);
    else
        write(fd, "out", 3);
        
    close(fd);
    
    gpioInfo[USER_TO_GPIO(gpio)].direction = direction;
    
    return 0;
}

int libGpioBitRead(int gpio, int *value)
{
    int fd;
    char val[3];
    
    if (!libGpioRangeCheck(gpio))
        return -1;    
        
    if (!gpioInfo[USER_TO_GPIO(gpio)].open)
        return -2;
    
    makeGpioPath(gpio, GPIO_PATH_VALUE);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    
    fd = open(gpioPathBuff, O_RDONLY);
    if (fd <= 0) {
        syslog(LOG_ERR, "Failed to open gpio value: gpio=%d, errno=%d\n", gpio, errno);
        
        pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
        return -3;
    }

    if (read(fd, val, 3) < 0) {
        syslog(LOG_ERR, "Failed to read gpio value: gpio=%d, errno=%d\n", gpio, errno);
        
        pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
        return -3;
    }
    close(fd);
 
    *value = atoi(val);
    gpioInfo[USER_TO_GPIO(gpio)].value = *value;
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);

    return 0;
}

int libGpioBitWrite(int gpio, int value)
{
    int fd;
    char val[3];
    
    if (!libGpioRangeCheck(gpio))
        return -1;    
        
    if (!gpioInfo[USER_TO_GPIO(gpio)].open) {
        syslog(LOG_ERR, "GPIO not open: gpio=%d, errno=%d\n", gpio, errno);
        return -2;
    }
        
    makeGpioPath(gpio, GPIO_PATH_VALUE);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    
    fd = open(gpioPathBuff, O_WRONLY);
    if (fd <= 0) {
        syslog(LOG_ERR, "Failed to open gpio value: gpio=%d, errno=%d\n", gpio, errno);
        
        pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
        return -3;
    }
    
    sprintf(val, "%d", value);
    if (write(fd, val, 3) < 0) {
        syslog(LOG_ERR, "Failed to wrte gpio value: gpio=%d, errno=%d\n", gpio, errno);
        
        pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
        return -3;
    }
 
    close(fd);
    gpioInfo[USER_TO_GPIO(gpio)].value = value;
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    
    return 0;
}

int libGpioStatus(int gpio, gpioStatus_t *info)
{
    int reading;
    
    if (!libGpioRangeCheck(gpio))
        return -1; 
    
    // do a reading for the status info - no point having stale values
    libGpioBitRead(gpio, &reading);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    memcpy(info, &gpioInfo[USER_TO_GPIO(gpio)], sizeof(gpioStatus_t));
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    return 0;
}
