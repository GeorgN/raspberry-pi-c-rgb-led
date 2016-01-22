#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "pwm.h"
#include "gpio.h"

/*
 * Pulse Width Modulation thread function. Pass a pointer to a GPIO_PWM_PARAMETERS struct.
 * Passing as a pointer means the duty cycle can be changed whilst the thread is executing.
 * Its software pulse width modulation so not very accurate!
 */
void *pwmCycleGpioPin(void *parameters)
{
    GPIO_PWM_PARAMETERS *pwmParameters = (GPIO_PWM_PARAMETERS *)parameters;

    int returnStatus = EXIT_SUCCESS;

    while(pwmParameters->terminate == false)
    {
        // Parameters can be changed outside of this thread, so need to re-calulate every iteration
        pwmParameters->microSecondsTotal = (int)(GPIO_PWM_UNIT_MICRO_SECOND / pwmParameters->cyclesPerSecond);
        pwmParameters->microSecondsHigh = pwmParameters->microSecondsTotal * (float)(pwmParameters->dutyCycle / GPIO_PWM_HUNDRED_PERCENT);
        pwmParameters->microSecondsLow = pwmParameters->microSecondsTotal - pwmParameters->microSecondsHigh;

        // Set pin high and sleep for high cycle
        if (pwmParameters->microSecondsHigh > 0)
        {
            returnStatus = gpioWrite(pwmParameters->gpioPin, GPIO_HIGH);
            assert(returnStatus == EXIT_SUCCESS);
            usleep(pwmParameters->microSecondsHigh);
        }

        // Set pin low and sleep for low cycle
        if (pwmParameters->microSecondsLow > 0)
        {
            returnStatus = gpioWrite(pwmParameters->gpioPin, GPIO_LOW);
            assert(returnStatus == EXIT_SUCCESS);
            usleep(pwmParameters->microSecondsLow);
        }
    }

    pthread_exit(NULL);
}
