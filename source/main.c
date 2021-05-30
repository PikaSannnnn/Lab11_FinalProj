/*	Author: sdong027
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "essentials.h"
#include "sevenseg.h"
#include "scheduler.h"
#include "lcd.h"

/*	MC PINS
	PA0-PA7	= LCD_Data
	PB0-PB3	= Shift Register
	PB4-PB5	= LCD_Control
	PB6	= Speaker
	PC0-PC7	= Keypad
	PD0-PD7	= 7-Segment
*/
/*	SHIFT REG PINS
	Q0
*/

int main(void) {
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;

	TimerSet(1000);	// use GCD function
	TimerOn();
	LCD_init();

	while (1) {
		LCD_ClearScreen();
		LCD_DisplayString(1, (const unsigned char *)("################"));
		LCD_DisplayString(17, (const unsigned char *)("################"));
		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
