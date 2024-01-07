#include "seg7_display.h"
#include "load.h"
#include "timer0.h"
#include "temp_sensor.h"

int main(void)
{
	initDisplay();
	initLoad();
	
	initTimer();
	resetTimer();
	
	while(1)
	{
		checkSensor();
	}
}