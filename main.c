#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "GPIO/pwm.h"
#include "GPIO/gpio.h"

#define GPIO_PIN_RED    GPIO_PIN_17
#define GPIO_PIN_GREEN  GPIO_PIN_27
#define GPIO_PIN_BLUE   GPIO_PIN_22
#define GPIO_PIN_VOLTS  GPIO_PIN_18

#define RED             0
#define GREEN           1
#define BLUE            2

#define COLOUR_RED      "Red"
#define COLOUR_GREEN    "Green"
#define COLOUR_BLUE     "Blue"
#define COLOUR_UNKNOWN  "?"

static int returnStatus;

static void initialiseGpioPins();
static void turnOffLed();
static void tidyUpGpioPins(const bool assertReturnStatus);
static void playWithLed(GPIO_PWM_PARAMETERS pwmParameters[]);
static const char* singleColour(const int gpioPin);

static void fadeSingleColourWithAnother(GPIO_PWM_PARAMETERS *pwmParametersFade, const int fadeNumberOfTimes, GPIO_PWM_PARAMETERS *pwmParametersConstant);
static void fadeSingleColourWithTwoOthers(GPIO_PWM_PARAMETERS *pwmParametersFade, const int fadeNumberOfTimes, GPIO_PWM_PARAMETERS *pwmParametersConstant1, GPIO_PWM_PARAMETERS *pwmParametersConstant2);
static void fadeSingleColour(GPIO_PWM_PARAMETERS *pwmParameters, const int fadeNumberOfTimes);

const int CYCLES_PER_SECOND = 100;      // 100 Hz
const int SINGLE_CYCLE_PER_SECOND = 1;

/*
 * Program main entry point
 */
int main()
{
    returnStatus = EXIT_SUCCESS;

    // Initialise gpio pins
    initialiseGpioPins();

    // Create an array for the RGB Led pins and set the initial values for each of the pins
    GPIO_PWM_PARAMETERS pwmParameters[3] =
    {
        {
            .gpioPin            = GPIO_PIN_RED,
            .cyclesPerSecond    = SINGLE_CYCLE_PER_SECOND,
            .dutyCycle          = GPIO_PWM_DUTY_CYCLE_HIGH
        },
        {
            .gpioPin            = GPIO_PIN_GREEN,
            .cyclesPerSecond    = SINGLE_CYCLE_PER_SECOND,
            .dutyCycle          = GPIO_PWM_DUTY_CYCLE_HIGH
        },
        {
            .gpioPin            = GPIO_PIN_BLUE,
            .cyclesPerSecond    = SINGLE_CYCLE_PER_SECOND,
            .dutyCycle          = GPIO_PWM_DUTY_CYCLE_HIGH
        }
    };

    // Create a thread for each of the colour pins
    returnStatus = pthread_create(&pwmParameters[RED].threadId, NULL, &pwmCycleGpioPin, (void*)&pwmParameters[RED]);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = pthread_create(&pwmParameters[GREEN].threadId, NULL, &pwmCycleGpioPin, (void*)&pwmParameters[GREEN]);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = pthread_create(&pwmParameters[BLUE].threadId, NULL, &pwmCycleGpioPin, (void*)&pwmParameters[BLUE]);
    assert(returnStatus == EXIT_SUCCESS);

    // Before applying voltage to Led wait for at least a single pass of each thread
    usleep(GPIO_PWM_UNIT_MICRO_SECOND / 2);

    // Apply voltage
    returnStatus = gpioWrite(GPIO_PIN_VOLTS, GPIO_HIGH);
    assert(returnStatus == EXIT_SUCCESS);

    // Play with the Led
    playWithLed(pwmParameters);

    // Terminate the pulse width modulation on each of the pins
    pwmParameters[RED].terminate = true;
    pwmParameters[GREEN].terminate = true;
    pwmParameters[BLUE].terminate = true;

    // Wait for the threads to terminate
    returnStatus = pthread_join(pwmParameters[RED].threadId, NULL);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = pthread_join(pwmParameters[GREEN].threadId, NULL);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = pthread_join(pwmParameters[BLUE].threadId, NULL);
    assert(returnStatus == EXIT_SUCCESS);

    // Turn off RGB Led
    turnOffLed();

    // Tidy up
    tidyUpGpioPins(true);

    return returnStatus;
}

/*
 * Initialise the GPIO pins for output
 */
void initialiseGpioPins()
{
    tidyUpGpioPins(false);

    returnStatus = gpioExportAndDirection(GPIO_PIN_RED, GPIO_OUTPUT);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioExportAndDirection(GPIO_PIN_GREEN, GPIO_OUTPUT);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioExportAndDirection(GPIO_PIN_BLUE, GPIO_OUTPUT);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioExportAndDirection(GPIO_PIN_VOLTS, GPIO_OUTPUT);
    assert(returnStatus == EXIT_SUCCESS);
}

/*
 * Turn off the Led
 */
void turnOffLed()
{
    returnStatus = gpioWrite(GPIO_PIN_VOLTS, GPIO_LOW);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioWrite(GPIO_PIN_RED, GPIO_LOW);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioWrite(GPIO_PIN_GREEN, GPIO_LOW);
    assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioWrite(GPIO_PIN_BLUE, GPIO_LOW);
    assert(returnStatus == EXIT_SUCCESS);
}

/*
 * Tidy up should be initialised
 */
void tidyUpGpioPins(const bool assertReturnStatus)
{
    returnStatus = gpioUnexport(GPIO_PIN_RED);
    if (assertReturnStatus == true) assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioUnexport(GPIO_PIN_GREEN);
    if (assertReturnStatus == true) assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioUnexport(GPIO_PIN_BLUE);
    if (assertReturnStatus == true) assert(returnStatus == EXIT_SUCCESS);

    returnStatus = gpioUnexport(GPIO_PIN_VOLTS);
    if (assertReturnStatus == true) assert(returnStatus == EXIT_SUCCESS);
}

/*
 * Return the colour for the GPIO Pin
 */
const char* singleColour(const int gpioPin)
{
    if (gpioPin == GPIO_PIN_RED)
        return COLOUR_RED;

    if (gpioPin == GPIO_PIN_GREEN)
        return COLOUR_GREEN;

    if (gpioPin == GPIO_PIN_BLUE)
        return COLOUR_BLUE;

    return COLOUR_UNKNOWN;
}

/*
 * Main function to change state of the Led
 */
void playWithLed(GPIO_PWM_PARAMETERS pwmParameters[])
{
    fadeSingleColourWithTwoOthers(&pwmParameters[RED],      3,  &pwmParameters[GREEN],  &pwmParameters[BLUE]);
    fadeSingleColourWithTwoOthers(&pwmParameters[GREEN],    3,  &pwmParameters[RED],    &pwmParameters[BLUE]);
    fadeSingleColourWithTwoOthers(&pwmParameters[BLUE],     3,  &pwmParameters[RED],    &pwmParameters[GREEN]);

    fadeSingleColourWithAnother(&pwmParameters[RED],        3, &pwmParameters[GREEN]);
    fadeSingleColourWithAnother(&pwmParameters[RED],        3, &pwmParameters[BLUE]);

    fadeSingleColourWithAnother(&pwmParameters[GREEN],      3, &pwmParameters[RED]);
    fadeSingleColourWithAnother(&pwmParameters[GREEN],      3, &pwmParameters[BLUE]);

    fadeSingleColourWithAnother(&pwmParameters[BLUE],       3, &pwmParameters[RED]);
    fadeSingleColourWithAnother(&pwmParameters[BLUE],       3, &pwmParameters[GREEN]);

    printf("Fading %s\n", singleColour(pwmParameters[RED].gpioPin));
    pwmParameters[RED].dutyCycle = GPIO_PWM_DUTY_CYCLE_LOW;
    fadeSingleColour(&pwmParameters[RED],   3);

    printf("Fading %s\n", singleColour(pwmParameters[GREEN].gpioPin));
    pwmParameters[GREEN].dutyCycle = GPIO_PWM_DUTY_CYCLE_HIGH;
    fadeSingleColour(&pwmParameters[GREEN], 3);

    printf("Fading %s\n", singleColour(pwmParameters[BLUE].gpioPin));
    pwmParameters[BLUE].dutyCycle = GPIO_PWM_DUTY_CYCLE_LOW;
    fadeSingleColour(&pwmParameters[BLUE],  3);
}

/*
 * Fade a Led colour with another
 */
void fadeSingleColourWithAnother(GPIO_PWM_PARAMETERS *pwmParametersFade, const int fadeNumberOfTimes, GPIO_PWM_PARAMETERS *pwmParametersConstant)
{
    printf("Fading %s with %s\n",
        singleColour(pwmParametersFade->gpioPin),
        singleColour(pwmParametersConstant->gpioPin));

    pwmParametersConstant->dutyCycle = GPIO_PWM_DUTY_CYCLE_LOW;

    fadeSingleColour(pwmParametersFade, fadeNumberOfTimes);

    pwmParametersConstant->dutyCycle = GPIO_PWM_DUTY_CYCLE_HIGH;
}

/*
 * Fade a Led colour with another
 */
void fadeSingleColourWithTwoOthers(GPIO_PWM_PARAMETERS *pwmParametersFade, const int fadeNumberOfTimes, GPIO_PWM_PARAMETERS *pwmParametersConstant1, GPIO_PWM_PARAMETERS *pwmParametersConstant2)
{
    printf("Fading %s with %s and %s\n",
        singleColour(pwmParametersFade->gpioPin),
        singleColour(pwmParametersConstant1->gpioPin),
        singleColour(pwmParametersConstant2->gpioPin));

    pwmParametersConstant1->dutyCycle = GPIO_PWM_DUTY_CYCLE_LOW;
    pwmParametersConstant2->dutyCycle = GPIO_PWM_DUTY_CYCLE_LOW;

    fadeSingleColour(pwmParametersFade, fadeNumberOfTimes);

    pwmParametersConstant1->dutyCycle = GPIO_PWM_DUTY_CYCLE_HIGH;
    pwmParametersConstant2->dutyCycle = GPIO_PWM_DUTY_CYCLE_HIGH;
}

/*
 * Fade a Led colour
 */
void fadeSingleColour(GPIO_PWM_PARAMETERS *pwmParameters, const int fadeNumberOfTimes)
{
    pwmParameters->cyclesPerSecond = CYCLES_PER_SECOND;

    int dutyCycle = pwmParameters->dutyCycle;
    bool increasing = (pwmParameters->dutyCycle == GPIO_PWM_DUTY_CYCLE_LOW);

    for (int count=0; count<(100 * fadeNumberOfTimes); count++)
    {
        if (increasing == true)
            dutyCycle += 2;
        else
            dutyCycle -= 2;

        pwmParameters->dutyCycle = dutyCycle;
        usleep(GPIO_PWM_UNIT_MICRO_SECOND * 0.05);

        if (increasing == true  && dutyCycle == GPIO_PWM_DUTY_CYCLE_HIGH) increasing = false;
        if (increasing == false && dutyCycle == GPIO_PWM_DUTY_CYCLE_LOW)  increasing = true;
    }

    // Turn off Led
    pwmParameters->cyclesPerSecond = SINGLE_CYCLE_PER_SECOND;
    pwmParameters->dutyCycle = GPIO_PWM_DUTY_CYCLE_HIGH;
}


