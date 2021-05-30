/*	Author: sdong027
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
/*	
	MC PINS
	PA0-PA7	= LCD_Data
	PB0-PB3	= Shift Register
	PB4-PB5	= LCD_Control
	PB6	= Speaker
	PC0-PC7	= Keypad
	PD0-PD7	= 7-Segment

	SHIFT REG PINS
	Q0
*/

#include <avr/io.h>
#include <time.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "essentials.h"
#include "sevenseg.h"
#include "scheduler.h"
#include "lcd.h"

#define GREEN	0x01
#define RED	0x02	

unsigned char difficulty = 0;	// 0 = none ; 1 = unsecure ; 2 = secure ; 3 = maximum security
unsigned char numCompleted = 0;	// resets at a lock unlock	// LED
unsigned char numUnlocks = 0;	// total number of unlocks, need 2 to open safe	// LED
unsigned char numAttempts = 0;	// total number of attempts, depends on difficulty
unsigned char timeAttempt = 0;	// total "time" value, multiplier to depends on difficulty	// LED
unsigned char regData;
const unsigned double frq = 0.00;
int score = 0;

enum DifficultyStates {};
void SetDifficultySM() {
	// outputs difficulties
	// sets selected difficulty
		// set numAttempts
		// set timeAttempt
	// difficulty chosen by cycling (and printing it out)
		// use A ^, C v, B select
}

enum MathStates {};
unsigned char MathProblemSM() {	// prints and checks math inputs
	static unsigned char solved;	// whether or not problem is solved, 0 => not yet solved (but not failed)
	unsigned char failed = 0x00;	// whether or not problem is incorrectly solved, 1 => not correct
	// convert char return to number from getKeypad
		// nums => numbers
		// Reds => enter
		// Output as typing
	// use switches to create math, math is rand

	return failed;	// return to do punishment
}
void ComputeScore() {

}

enum SafeStates {};
void SafeSM() {	// main sm, handles locked, unlocked, and in betweens (and fail)
	
}

int main(void) {
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;

	int randNum;
	int numPeriod = 0; 
	unsigned char gameStarted = 0x00;
	TimerSet(1000);	// use GCD function
	TimerOn();
	LCD_init();

	static task safe;
	task *tasks[] = {&safe};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char start = -1;

	safe.state = start;	// rename task name
	safe.period = 50;
	safe.elapsedTime = safe.period;
	safe.TickFct = &SafeSM;

	while (1) {
		if ((!gameStarted) && (!difficulty)) {	// game not started and difficulty not selected
			numPeriod++;
		}
		else if ((!gameStarted) && difficulty) {	// difficulty has been selected, seed rand
			randNum = (difficulty * numPeriod * 3) % 7;
			srand(randNum);
			gameStarted = 0xFF;	// sets gameStarted to true, prevents this if-else from running
		}
		LCD_ClearScreen();
		LCD_DisplayString(1, (const unsigned char *)("################"));
		LCD_DisplayString(17, (const unsigned char *)("################"));
		transmit_data(regData);	// sends data to register and immediately pushes
		for (unsigned short i = 0; i < numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 25;
		}

		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
