#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "input.h"
#include "gpio.h"

/*
 * Thread which polls the input pin then when the value changed calls the function pointer
 */
void *inputPollGpioPin(void *parameters)
{
    GPIO_INPUT_PARAMETERS *inputParameters = (GPIO_INPUT_PARAMETERS *)parameters;

    // Validate GPIO pin
    assert(gpioIsValidPin(inputParameters->gpioPin) == true);

    int returnStatus = EXIT_SUCCESS;

    // Get initial value of GPIO pin
    int lastValue = 0;
    returnStatus = gpioRead(inputParameters->gpioPin, &lastValue);
    assert(returnStatus == EXIT_SUCCESS);

    while(inputParameters->terminate == false)
    {
        // Read GPIO pin
        returnStatus = gpioRead(inputParameters->gpioPin, &inputParameters->inputValue);
        assert(returnStatus == EXIT_SUCCESS);

        // If value of GPIO pin changed then call the input value changed function
        if (lastValue != inputParameters->inputValue)
        {
            inputParameters->inputValueChanged(inputParameters->inputValue);
        }

        // Keep value and sleep
        lastValue = inputParameters->inputValue;
        usleep(inputParameters->sleepDelay);
    }

    pthread_exit(NULL);
}
