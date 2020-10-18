#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "io.h"

//time the relays will be energized before powering off again
const uint16_t powerOnTime = 60 * 60 * 5;

// time in ms to wait before re-registering a button press for debouncing
const uint8_t debounceDelay = 100;

//time the relays will still be energized (in seconds)
volatile uint16_t timeUntilPowerDown = 0;
volatile bool btnPressed = false;

avr::Input button(avr::PortB, PB0, true, true);
avr::Output relay2(avr::PortB, PB1);
avr::Output relay1(avr::PortB, PB2);
avr::Output addon(avr::PortB, PB3);
avr::Output led(avr::PortB, PB4);

void setupTimer1()
{
    // Setup the Timer1 to fire once a second
    cli();
    // prescaler = FCPU / 16384 ~= 61Hz
    // CTC - clear on compare match with OCR1C
    TCCR1 |= (1 << CTC1);
    TCCR1 |= (1 << CS10) | (1 << CS11) | (1 << CS12) | (1 << CS13);

    // (F_CPU / PRESCALER ) / 1Hz ~= 61
    OCR1A = 61;
    OCR1C = OCR1A;

    // Output Compare Match A Interrupt Enable
    TIMSK |= (1 << OCIE1A);

    // enable interrupts
    sei();
}

void setupInterrupt()
{
    cli();
    // pin change interrupt enable
    GIMSK |= (1 << PCIE);

    // pin change interrupt enabled for PCINT4
    PCMSK |= (1 << PCINT0);

    // enable interrupts
    sei();
}

void debounceButton()
{
    //wait fo the debounce delay before resetting the button state
    _delay_ms(debounceDelay);
    btnPressed = false;
}

int main()
{
    setupTimer1();
    setupInterrupt();

    while (true)
    {
        if (btnPressed)
        {
            if (timeUntilPowerDown == 0)
            {
                // outputs were off, time to switch them on
                led << 1;
                relay1 << 1;
                relay2 << 1;
                timeUntilPowerDown = powerOnTime;
            }
            else
            {
                //switch off the led to indicate we registered the press
                led << 0;
                // outputs are on. start a countdown for power off
                timeUntilPowerDown = 2;

                debounceButton();

                // if the button is pressed again in the next second
                // this was a double press, so rearm the timer
                _delay_ms(1000);
                if (btnPressed)
                {
                    led << 1;
                    timeUntilPowerDown = powerOnTime;
                }
                else
                {
                    //it was not a double-press so interpret it as
                    //a request to power down. (the next if statement
                    // will actually handle the power-down so we don't
                    // need to to anything here)
                }
            }
            debounceButton();
        }

        if (timeUntilPowerDown == 0)
        {
            led << 0;
            relay1 << 0;
            relay2 << 0;
        }
    }
}

ISR(PCINT0_vect)
{
    //only trigger on a high-level of the button
    if (button)
    {
        btnPressed = true;
    }
}

ISR(TIMER1_COMPA_vect)
{
    //decrement the coundown timer
    if (timeUntilPowerDown > 0)
    {
        timeUntilPowerDown--;
    }
}
