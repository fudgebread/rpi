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
#include <stdint.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "../include/lib_gpio.h"

#define GPIO_PATH_EXPORT "/sys/class/gpio/export"
#define GPIO_PATH_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_PATH "/sys/class/gpio/gpio%d"
#define GPIO_PATH_DIRECTION "/sys/class/gpio/gpio%d/direction"
#define GPIO_PATH_VALUE "/sys/class/gpio/gpio%d/value"
#define GPIO_PATH_BUFF_MAX 40
#define USER_TO_GPIO(a) (a-GPIO_MIN)

/* Addresses */
#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

/* 4K memory? */
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
char *gpio_mem, *gpio_map;
char *spi0_mem, *spi0_map;

//0x 7E20 0000      GPFSEL0     GPIO Function Select 0

// I/O access
volatile uint32_t *memMappedGPIO;

/* Buffer for memMappedGPIO path manipulation */
static char gpioPathBuff[GPIO_PATH_BUFF_MAX];

/* GPIO info array */
static gpioStatus_t gpioInfo[GPIO_TOTAL];

/* Whether or not we have initialised the info array */
static int initialised = 0;

static pthread_mutex_t gpioMutexes[GPIO_TOTAL];
static pthread_mutex_t memMapMutex;

/**
 * MAP
 * 
 * 
 * Offset       name        purpose
 * 0            sel0        function select 0
 * 4            sel1        function select 1
 * 8            sel2        function select 2
 * c            sel3        function select 3
 * 10           sel4        function select 4
 * 14           sel5        function select 5
 * 
 * 1c           set0        pin output set 0
 * 20           set1        pin output set 1
 * 
 * 28           clr0        pin output clear 0
 * 2c           clr1        pin output clear 1
 * 
 * 34           lev0        pin level 0
 * 38           lev1        pin level 1
 * 
 * 40           eve0        pin event detect status 0
 * 44           eve1        pin event detect status 1
 * 
 * 4c           ris0        rising edge detect enable 0
 * 50           ris1        rising edge detect enable 1
 * 
 * 58           fal0        falling edge detect enable 0
 * 5c           fal0        falling edge detect enable 1
 * 
 * 64           hen0        high detect anable 0
 * 68           hen1        high detect enable 1
 * 
 * 70           len0        low detect enable 0
 * 74           len1        low detect enable 1
 * 
 * 7c           are0        async rising edge detect 0
 * 80           are1        
 * 
 * 88           afe0
 * 8c           afe1
 * 
 * 94           pue0        pull-up/down enable
 * 98           pueclk0     pullup/down enable clock 0
 * 9c           pueclk1     pullup/down enable clock 1    
 * 
 **/

typedef enum gpioAddr {
    /* function select */
    gpioAddr_gpFsel0 = 0,
    gpioAddr_gpFsel1 = 1,
    gpioAddr_gpFsel2 = 2,
    gpioAddr_gpFsel3 = 3,
    gpioAddr_gpFsel4 = 4,
    gpioAddr_gpFsel5 = 5,
    /* */
    gpioAddr_gpSet0 = 7,
    gpioAddr_gpSet1 = 8,
    /* */
    gpioAddr_gpClr0 = 10,
    gpioAddr_gpClr1 = 11,
    /* */
    gpioAddr_gpLev0 = 13,
    gpioAddr_gpLev1 = 14,
    /* */
    gpioAddr_gpEds0 = 16,
    gpioAddr_gpEds1 = 17,
    /* */
    gpioAddr_gpRen0 = 19,
    gpioAddr_gpRen1 = 20,
    /* */
    gpioAddr_gpFen0 = 22,
    gpioAddr_gpFen1 = 23,
    /* */
    gpioAddr_gpHen0 = 25,
    gpioAddr_gpHen1 = 26,
    /* */
    gpioAddr_gpLen0 = 28,
    gpioAddr_gpLen1 = 29,
    /* */
    gpioAddr_gpAren0 = 31,
    gpioAddr_gpAren1 = 32,
    /* */
    gpioAddr_gpAfen0 = 34,
    gpioAddr_gpAfen1 = 35,
    /* */
    gpioAddr_gpPud = 37,
    gpioAddr_gpPudClk0 = 38,
    gpioAddr_gpPudClk1 = 39
} gpioAddr_t ;

/***********************************************************************
 * 
 * Local Functions
 * 
 **********************************************************************/
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
int libGpioRangeCheck(int gpio)
{
    if (gpio < GPIO_MIN || gpio > GPIO_MAX) {
        syslog(LOG_ERR, "Gpio out of range: gpio=%d\n", gpio);
        return 0;
    }
    return 1;
}

/**
 * Sysfs
 **/
void libGpioSysFsInit(int gpio)
{
    int i, j;
    
    if (initialised)
        return;
        
    for (i=0, j=GPIO_MIN; i<GPIO_TOTAL; i++, j++) {
        memset(&gpioInfo[i], 0, sizeof(gpioStatus_t));
        gpioInfo[i].gpio = j;
        gpioInfo[i].open = 0;
        
        pthread_mutex_init(&gpioMutexes[i], NULL);
    }
    initialised = 1;
}

int libGpioSysFsOpen(int gpio)
{
    char tmpBuff[4];
    int fd;
    
    if (!libGpioRangeCheck(gpio))
        return -1;        
    
    if (gpioInfo[USER_TO_GPIO(gpio)].open) {
        syslog(LOG_ERR, "GPIO already open: gpio=%d, errno=%d\n", gpio, errno);
        return 0;
    }
    
    // echo [memMappedGPIO] > export
    fd = open(GPIO_PATH_EXPORT, O_WRONLY);
    if (fd <= 0) {
        syslog(LOG_ERR, "Failed to export gpio: memMappedGPIO=%d, errno=%d\n", gpio, errno);
        return -3;
    }
    
    sprintf(tmpBuff, "%d", gpio);
    write(fd, tmpBuff, 4);
    close(fd);
    
    gpioInfo[USER_TO_GPIO(gpio)].open = 1;
    
    return 0;
}

int libGpioSysFsClose(int gpio)
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
    
    // echo [memMappedGPIO] > unexport
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

int libGpioSysFsDirection(int gpio, int direction)
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

int libGpioSysFsBitRead(int gpio, int *value)
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

int libGpioSysFsBitWrite(int gpio, int value)
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
    libGpioSysFsBitRead(gpio, &reading);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    memcpy(info, &gpioInfo[USER_TO_GPIO(gpio)], sizeof(gpioStatus_t));
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    return 0;
}

/**
 * Memory-mapped GPIO
 **/
int libGpioMemMapSet(int gpio)
{
    int shift = 0;
    int offset = 0;
    
    if (!libGpioRangeCheck(gpio))
        return -1; 
        
    if (gpio >= 0 && gpio <= 31) {
        offset = gpioAddr_gpSet0;
        shift = gpio;
    }
    else if (gpio >= 32 && gpio <= 53) {
        offset = gpioAddr_gpSet1;
        shift = gpio - 32;
    }
    else {
        return -1;
    }
    
    pthread_mutex_lock(&memMapMutex);
    *(memMappedGPIO + offset) |= (1<<shift);
    pthread_mutex_unlock(&memMapMutex);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    gpioInfo[USER_TO_GPIO(gpio)].value = 1;
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    
    return 0;
}

int libGpioMemMapClear(int gpio)
{
    int shift = 0;
    int offset = 0;
    
    if (!libGpioRangeCheck(gpio))
        return -1; 
        
    if (gpio >= 0 && gpio <= 31) {
        offset = gpioAddr_gpClr0;
        shift = gpio;
    }
    else if (gpio >= 32 && gpio <= 53) {
        offset = gpioAddr_gpClr1;
        shift = gpio - 32;
    }
    else {
        return -1;
    }
    
    pthread_mutex_lock(&memMapMutex);
    *(memMappedGPIO + offset) |= (1<<shift);
    pthread_mutex_unlock(&memMapMutex);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    gpioInfo[USER_TO_GPIO(gpio)].value = 0;
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    return 0;
}


int libGpioMemMapRead(int gpio, int *value)
{
    uint32_t mask = 0;
    int offset = 0;
    uint32_t reading = 0;
    
    if (!libGpioRangeCheck(gpio))
        return -1; 
        
    if (gpio >= 0 && gpio <= 31) {
        offset = gpioAddr_gpLev0;
        mask = 1<<gpio;
    }
    else if (gpio >= 32 && gpio <= 53) {
        offset = gpioAddr_gpLev1;
        mask = 1<<(gpio - 32);
    }
    else {
        return -1;
    }
    
    pthread_mutex_lock(&memMapMutex);
    reading = *(memMappedGPIO + offset);
    pthread_mutex_unlock(&memMapMutex);

    reading &= mask;
    *value = reading > 0 ? 1 : 0;
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    gpioInfo[USER_TO_GPIO(gpio)].value = *value;
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    
    return 0;
}

/**
 * Each gpio has 3 bits a function select register.
 * So we need to:
 *  find the value that does the function select
 *      already passed in
 *  shift that by gpio*3
 **/
int libGpioMemMapSelect(int gpio, gpioSel_t sel, int set)
{
    int bitShift = 0;
    unsigned int offset = 0;
    volatile unsigned int *addr = memMappedGPIO;
    
    if (!libGpioRangeCheck(gpio))
        return -1; 
        
    if (gpio >= 0 && gpio <= 9) {
        bitShift = (gpio*3);
        offset = gpioAddr_gpFsel0;
    }
    else if (gpio >= 10 && gpio <= 19) {
        bitShift = (gpio%10)*3;
        offset = gpioAddr_gpFsel1;
    }
    else if (gpio >= 20 && gpio <= 29) {
        bitShift = (gpio%20)*3;
        offset = gpioAddr_gpFsel2;
    }
    else if (gpio >= 30 && gpio <= 39) {
        bitShift = (gpio%30)*3;
        offset = gpioAddr_gpFsel3;
    }
    else if (gpio >= 40 && gpio <= 49) {
        bitShift = (gpio%40)*3;
        offset = gpioAddr_gpFsel4;
    }
    else if (gpio >= 50 && gpio <= 53) {
        bitShift = (gpio%50)*3;
        offset = gpioAddr_gpFsel5;
    }
    else {
        return -1;
    }

    if (set)
        *(memMappedGPIO + offset) |= (sel<<bitShift);
    else
        *(memMappedGPIO + offset) &= ~(sel<<bitShift);
    
    pthread_mutex_lock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    gpioInfo[USER_TO_GPIO(gpio)].direction = sel;
    
    if (!set && sel == gpioSel_alt3)
        gpioInfo[USER_TO_GPIO(gpio)].open = 0;
    else
        gpioInfo[USER_TO_GPIO(gpio)].open = 1;
        
    pthread_mutex_unlock(&gpioMutexes[USER_TO_GPIO(gpio)]);
    
    return 0;
}

// from: https://www.elinux.org/Rpi_Datasheet_751_GPIO_Registers
int libGpioMemMapInit()
{
    pthread_mutex_init(&memMapMutex, NULL);
    
    unsigned long pageSize = sysconf(_SC_PAGE_SIZE);
   /**
    *  open /dev/mem 
    * /dev/mem is the physical memory of the device. So you can access
    * memory mapped peripherals from here.
    * /dev/mem is a char device
    * 
    * Also read this is a quick and dirty way of using devices without
    * the kernel
    **/
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        syslog(LOG_ERR, "Cannot open /dev/mem (%d)\n", errno);
        return -1;
   }

   /**
    * Allocate MAP block. This allocates 4096 + 4095 = 8191. Since we 
    * need an allocation on the page boundary, 2 pages are allocated and
    * the starting address adjusted based on the page boundary (if
    * required).
    **/
   
   if ((gpio_mem = malloc(BLOCK_SIZE + pageSize-1)) == NULL) {
      syslog(LOG_ERR, "Could not allocate mems (%d)\n", errno);
      return -1;
   }
   
   /* We want an aligned page. So if the address of our virtual mem isn't
    * aligned on a page, we want to shift the starting address of it such
    * that it is aligned. This must be the reason for allocating double what
    * we need: you're guaranteed to get an aligned page if you have page+something
    * allocated ;)
    * 
    * However, you'd probably want to keep a pointer to the original alloc
    * so you can free it. No?
    */
    if ((unsigned long)gpio_mem % pageSize) {
        syslog(LOG_WARNING, "Aligning virtual memory\n");
        gpio_mem += pageSize - ((unsigned long)gpio_mem % pageSize);
    }

   // Now map it
   gpio_map = (unsigned char *)mmap( /* Byte addressable */
      (caddr_t)gpio_mem,    /* map to the virtual memory we allocated */
      BLOCK_SIZE,           /* the mapping is this size (4096)*/
      PROT_READ|PROT_WRITE, /* read/write access to pages*/
      MAP_SHARED|MAP_FIXED, /* share this mapping (updates visible to other processes
                                FIXED==don't interpret addr as a hint*/
      mem_fd,               /* The actual /dev/mem */
      GPIO_BASE             /* offset into mem_fd (/dev/mem) */
   );

   if ((long)gpio_map < 0) {
      syslog(LOG_ERR, "mmap failed (%d)\n", errno);
      return -1;
   }

    // Always use volatile pointer!
    memMappedGPIO = (volatile unsigned *)gpio_map;

    return 0;
}
