#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "seg7_display.h"
#include "load.h"

#define BYTE		8
#define TEMP_MID	0x0800
#define TEMP_INC	(0.0625)

#define SKIP_ROM	0xCC
#define CONVERT_T	0x44
#define READ_SP		0xBE

#define DECIMAL		10
#define TEMP_ON		15
#define TEMP_OFF	20

bool negativeTemperatureFlag = false;

static bool sensorDetected(void);
static uint8_t readByteSensor(void);
static void writeByteSensor(const uint8_t);
static int16_t readDataSensor(void);
static int16_t convertToCelsius(const int16_t);

void checkSensor(void)
{
	extern uint8_t digits[];
	extern const uint8_t DIGITS_SIZE;
	
	if(sensorDetected())
	{
		int16_t temp = convertToCelsius(readDataSensor());
		negativeTemperatureFlag = temp < 0 ? true : false;
		
		if(temp < TEMP_ON * DECIMAL)
			turnLoadOn();
		if(temp > TEMP_OFF * DECIMAL)
			turnLoadOff();
		
		fillDigits(digits, DIGITS_SIZE, temp, negativeTemperatureFlag);
	}
}

static bool sensorDetected(void)
{
	/*store SREG value and disable global interrupts*/
	uint8_t sreg = SREG;
	cli();
	
	/*bus master pull low*/
	DDRC |= _BV(DDC1);
	_delay_us(500);
	
	/*bus master release, resistor pull up*/
	DDRC &= ~_BV(DDC1);
	_delay_us(120);
	
	/*bus low -> sensor detected*/
	bool status = bit_is_clear(PINC, PINC1) ? true : false;
	_delay_us(380);
	
	/*restore SREG value*/
	SREG = sreg;
	
	return status;
}

static uint8_t readByteSensor(void)
{
	/*store SREG value and disable global interrupts*/
	uint8_t sreg = SREG;
	cli();
	
	uint8_t dataByte = 0;
	
	for(uint8_t i = 0; i < BYTE; ++i)
	{
		/*bus master pull low*/
		DDRC |= _BV(DDC1);
		_delay_us(2);
		
		/*bus master release, resistor pull up*/
		DDRC &= ~_BV(DDC1);
		_delay_us(13);
		
		dataByte |= bit_is_set(PINC, PINC1) ? _BV(i) : 0;
		_delay_us(45);
	}
	
	/*restore SREG value*/
	SREG = sreg;
	
	return dataByte;
}

static void writeByteSensor(const uint8_t cmdByte)
{
	/*store SREG value and disable global interrupts*/
	uint8_t sreg = SREG;
	cli();
	
	for(uint8_t i = 0; i < BYTE; ++i)
	{
		if(cmdByte & _BV(i))
		{
			/*bus master pull low*/
			DDRC |= _BV(DDC1);
			_delay_us(2);
			
			/*bus master release, resistor pull up*/
			DDRC &= ~_BV(DDC1);
			_delay_us(58);
		}
		else
		{
			/*bus master pull low*/
			DDRC |= _BV(DDC1);
			_delay_us(65);
			
			/*bus master release, resistor pull up*/
			DDRC &= ~_BV(DDC1);
			_delay_us(2);
		}
	}
	
	/*restore SREG value*/
	SREG = sreg;
}

static int16_t readDataSensor(void)
{
	int16_t data = 0;
	
	/*initialization sequence*/
	if(sensorDetected())
	{
		writeByteSensor(SKIP_ROM);
		writeByteSensor(CONVERT_T);
		_delay_ms(750);					/*12-bit resolution delay*/
		
		/*initialization sequence*/
		if(sensorDetected())
		{
			writeByteSensor(SKIP_ROM);
			writeByteSensor(READ_SP);
			
			data = readByteSensor() | (readByteSensor() << BYTE);
		}
	}
	
	return data;
}

static int16_t convertToCelsius(const int16_t tempData)
{
	/*positive temperature if true, negative otherwise*/
	return tempData <= TEMP_MID
		? tempData * TEMP_INC * DECIMAL
		: (~tempData + 1) * TEMP_INC * DECIMAL;
}
