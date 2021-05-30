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

int main(void) {
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;	// keyPad
	DDRD = 0xFF; PORTD = 0x00;

	TimerSet(250);
	TimerOn();

	while (1) {
		LCD_DisplayString(1, (const unsigned char*)"H");
		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
