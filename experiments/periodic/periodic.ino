#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define LED1 7

// This periodic task runs every 10ms
// It blinks the LED on or off every 1s
ISR(TIMER3_OVF_vect)
{
	static int count = 0;
	static bool led_on = false;

	// after 100 10ms intervals, toggle the LED
	count++;
	if (count == 99) {
		count = 0;
		led_on = !led_on;
		digitalWrite(LED1, led_on?HIGH:LOW);
	}

	// reset counter so it can fire again
	TCNT3 = 64910; // reload counter value
	TIFR3 = 0; // clear interrupt flag
}

void setup()
{
	pinMode(LED1, OUTPUT);

	// Configure Timer/Counter #3 (#1 is used by Arduino for delay() and friends)
	// This setup will cause it to count down to zero, triggering a TIMER3_OVF (timer 3 overflow) interrupt
	TCCR3B = 0; // disable timer/counter during configuration
	TIFR3 = 0; // clear overflow flag
	TCCR3A = 0; // set normal counter mode
	TCCR3B = 0x04; // set prescaler to 256. 16MHz/256 = 75kHz (16us period)
	TCNT3 = 65536-625; // pre-load counter.. 625 * 16us = 10ms
	TIMSK3 = 0x01; // enable timer interrupt
}

void loop()
{
}
