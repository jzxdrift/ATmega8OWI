#include <avr/io.h>

void initLoad(void)
{
	DDRB |= _BV(DDB0);
}

void turnLoadOn(void)
{
	PORTB |= _BV(PB0);
}

void turnLoadOff(void)
{
	PORTB &= ~_BV(PB0);
}
