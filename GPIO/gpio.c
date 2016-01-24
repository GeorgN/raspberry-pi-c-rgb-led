#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "gpio.h"

static char *GPIO_SYS_CLASS = "/sys/class/gpio";
static bool GPIO_VERBOSE = false;
static const char *DIRECTION_INSTRUCTION[] = { "in", "out" };

static char* systemFile(const char *systemFile);
static char* systemPinFile(const int pin, const char *systemFile);
static int systemFileWrite(char *systemFile, int bytes, char *buffer);
static bool isValidGPIOPin(const int pin);
static bool isValidGPIODirection(const int direction);
static bool isValidGPIOValue(const int value);

/*
 * Initialises GPIO pin, bash equivalent for pin 17...
 *
 * $ echo "17" > /sys/class/gpio/export
 */
int gpioExport(const int pin)
{
    if (isValidGPIOPin(pin) == false)
        return EXIT_FAILURE;

    char buffer[3];
    int bytes = snprintf(buffer, 3, "%d", pin);

    char *gpioSysFile = systemFile(GPIO_EXPORT);

    return systemFileWrite(gpioSysFile, bytes, buffer);
}

/*
 * Sets direction of GPIO pin, bash equivalent for pin 17...
 *
 * $ echo "out" > /sys/class/gpio/gpio17/direction
 * $ echo "in" > /sys/class/gpio/gpio17/direction
 */
int gpioDirection(const int pin, const int direction)
{
    if (isValidGPIOPin(pin) == false || isValidGPIODirection(direction) == false)
        return EXIT_FAILURE;

    char buffer[4];
    int bytes = snprintf(buffer, 4, "%s", DIRECTION_INSTRUCTION[direction]);

    char *gpioSysFile = systemPinFile(pin, GPIO_DIRECTION);

    return systemFileWrite(gpioSysFile, bytes, buffer);
}

/*
 * Initialises and sets direction of GPIO pin, bash equivalent for pin 17...
 *
 * $ echo "17" > /sys/class/gpio/export
 * $ echo "out" > /sys/class/gpio/gpio17/direction
 */
int gpioExportAndDirection(const int pin, const int direction)
{
    if (gpioExport(pin) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (gpioDirection(pin, direction) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/*
 * Sets GPIO pin to high or low, bash equivalent for pin 17...
 *
 * $ echo "1" > /sys/class/gpio/gpio17/value
 * $ echo "0" > /sys/class/gpio/gpio17/value
 */
int gpioWrite(const int pin, const int value)
{
    if (isValidGPIOPin(pin) == false || isValidGPIOValue(value) == false)
        return EXIT_FAILURE;

    char buffer[2];
    int bytes = snprintf(buffer, 2, "%d", value);

    char *gpioSysFile = systemPinFile(pin, GPIO_VALUE);

    return systemFileWrite(gpioSysFile, bytes, buffer);
}

/*
 * Un-Initialises (tidy-up) GPIO pin, bash equivalent for pin 17...
 *
 * $ echo "17" > /sys/class/gpio/unexport
 */
int gpioUnexport(const int pin)
{
    if (isValidGPIOPin(pin) == false)
        return EXIT_FAILURE;

    char buffer[3];
    int bytes = snprintf(buffer, 3, "%d", pin);

    char *gpioSysFile = systemFile(GPIO_UNEXPORT);

    return systemFileWrite(gpioSysFile, bytes, buffer);
}

/*
 * Change the root system directory (the default is the correct value for use).  Used for unit testing.
 */
void gpioTestMode(char *gpioSysClass)
{
    GPIO_SYS_CLASS = gpioSysClass;
}

/*
 * Set the verbose mode, so will send to standard output.  Used for unit testing and diagnostics.
 */
void gpioVerbose(const bool verbose)
{
    GPIO_VERBOSE = verbose;
}

/*
 * Build the path and file name
 */
static char* systemFile(const char *systemFile)
{
    char *gpioFile = malloc(sizeof(char) * (strlen(GPIO_SYS_CLASS) + strlen(systemFile) + 1));
    strcat(gpioFile, GPIO_SYS_CLASS);
    strcat(gpioFile, systemFile);
    return gpioFile;
}

/*
 * Build the path (including pin directory) and file name
 */
static char* systemPinFile(int pin, const char *systemFile)
{
    char systemDir[8];
    snprintf(systemDir, 8, GPIO_PIN_DIR, pin);

    char *gpioFile = malloc(sizeof(char) * (strlen(GPIO_SYS_CLASS) + strlen(systemDir) + strlen(systemFile) + 1));
    strcat(gpioFile, GPIO_SYS_CLASS);
    strcat(gpioFile, systemDir);
    strcat(gpioFile, systemFile);
    return gpioFile;
}

/*
 * Write to the system file
 */
static int systemFileWrite(char *gpioSystemFile, const int bytes, char *buffer)
{
    if (GPIO_VERBOSE) printf("Writing '%s' to %s\n", buffer, gpioSystemFile);

    int fileDescriptor = open(gpioSystemFile, O_WRONLY);
    if (fileDescriptor == -1)
    {
        if (GPIO_VERBOSE) printf("Error opening system file %s\n", gpioSystemFile);
        return EXIT_FAILURE;
    }

    if (write(fileDescriptor, buffer, bytes) == -1)
    {
        if (GPIO_VERBOSE) printf("Error writing system file %s\n", gpioSystemFile);
        close(fileDescriptor);
        return EXIT_FAILURE;
    };

    close(fileDescriptor);

    return EXIT_SUCCESS;
}

/*
 * Validate the GPIO pin number
 */
static bool isValidGPIOPin(const int pin)
{
    return (pin < GPIO_PIN_02 || pin > GPIO_PIN_27) ? false : true;
}

/*
 * Validate the GPIO pin direction
 */
static bool isValidGPIODirection(const int direction)
{
    return (direction == GPIO_OUTPUT || direction == GPIO_INPUT) ? true : false;
}

/*
 * Validate the GPIO pin value
 */
static bool isValidGPIOValue(const int value)
{
    return (value == GPIO_HIGH || value == GPIO_LOW) ? true : false;
}
