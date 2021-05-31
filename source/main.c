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

unsigned char difficulty = 1;	// 0 = none ; 1 = unsecure ; 2 = secure ; 3 = maximum security
unsigned char numCompleted = 0;	// resets at a lock unlock	// LED
unsigned char numUnlocks = 0;	// total number of unlocks, need 2 to open safe	// LED
unsigned char numAttempts = 0;	// total number of attempts, depends on difficulty
unsigned char timeAttempt = 0;	// total "time" value, multiplier to depends on difficulty	// LED
unsigned char failed = 0x00;	// whether or not problem is incorrectly solved, 1 => not correct
unsigned char input;
unsigned char regData;
unsigned char displayColumn = 1;
const double frq = 0.00;
int score = 0;

int SetDifficultySM(int state);
int MathProblemSM(int state);
int SafeSM(int state);	// main sm, handles locked, unlocked, and in betweens (and fail)

int Input(int state);
char* num_to_str(int number);
int text_to_num (unsigned char math);
void PrintText(char* text);
void ComputeScore();

int main(void) {
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;

	int randNum;
	int numPeriod = 0; 
	unsigned char gameStarted = 0xFF;			///////////////////////////////////////////
	TimerSet(1);	// MathSM will update/change at 1ms per update (make it seem instant)
	TimerOn();
	LCD_init();
	transmit_data(0x00);	// "Clear" register
	LCD_ClearScreen();

	static task safe, math, keyin;
	task *tasks[] = {&safe, &math, &keyin};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char start = -1;

	safe.state = start;
	safe.period = 50;
	safe.elapsedTime = safe.period;
	safe.TickFct = &SafeSM;

	math.state = start;
	math.period = 1;
	math.elapsedTime = math.period;
	math.TickFct = &MathProblemSM;

	keyin.state = start;
	keyin.period = 50;
	keyin.elapsedTime = keyin.period;
	keyin.TickFct = &Input;

	srand(0);					/////////////////////////////////////////////////////

	while (1) {
		if ((!gameStarted) && (!difficulty)) {	// game not started and difficulty not selected
			numPeriod++;
		}
		else if ((!gameStarted) && difficulty) {	// difficulty has been selected, seed rand
			randNum = (difficulty * numPeriod * 3) % 7;
			srand(randNum);
			gameStarted = 0xFF;	// sets gameStarted to true, prevents this and above if from running
		}
		else if (gameStarted) {
			transmit_data(0x00);	// sends data to register and immediately pushes
			for (unsigned short i = 0; i < numTasks; i++) {
				if (tasks[i]->elapsedTime == tasks[i]->period) {
					tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
					tasks[i]->elapsedTime = 0;
				}
				tasks[i]->elapsedTime += 1;	
			}
		}
		

		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}

enum DifficultyStates {EASY, MEDIUM, HARD, SELECT};
int SetDifficultySM(int state) {
	// outputs difficulties
	// sets selected difficulty
		// set numAttempts
		// set timeAttempt
	// difficulty chosen by cycling (and printing it out)
		// use A ^, C v, B select
	return state;
}

enum InputStates {WAIT, WAIT_RELEASE};
int Input(int state) {
	// get input and store it -> handles single press only.
	unsigned char keypadIn = GetKeypadKey();

	switch (state) {
		case WAIT:
			if (keypadIn != '\0') {
				state = WAIT_RELEASE;
				input = keypadIn;
			}
			break;
		case WAIT_RELEASE:
			if (keypadIn == '\0') {
				state = WAIT;
			}
			input = '\0';
			break;
		default:
			state = WAIT;
			input = 0x00;
			break;
	}
	return state;
}

enum MathStates {MATH_CLEAR, FIRSTNUM, NUMBER, OPERATOR, PRINT, SOLVE, CHECK};
int MathProblemSM(int state) {	// prints and checks math inputs
	static unsigned char solved;	// whether or not problem is solved, 0 => not yet solved (but not failed)
	static int Solution;		// solution to math
	static unsigned short equationLen;
	static int InputSolution;	// solution to input
	static char* operator = "\0";
	static short numOps;
	int randVal;

	switch (state) {
		case MATH_CLEAR:
			state = FIRSTNUM;
			break;
		case FIRSTNUM:
			state = OPERATOR;
			break;
		case NUMBER:
			if (numOps < difficulty) {
				state = OPERATOR;
			}
			else {
				state = PRINT;
			}
			break;
		case OPERATOR:
			state = NUMBER;
			break;
		case PRINT:
			state = SOLVE;
			break;
		case SOLVE:
			if (input == 'A' || input == 'B' || input == 'C' || input == 'D' || input == '#' || input == '*') {
				state = CHECK;
			}
			break;
		case CHECK:
			if (failed) {
				state = SOLVE;
			}
			break;
		default:
			state = MATH_CLEAR;
			solved = 0;
			failed = 0;
			Solution = 0;
			InputSolution = 0;
			equationLen = 0;
			numOps = 0;
			break;
	}

	switch (state) {
		case MATH_CLEAR:
			Solution = 0;
			LCD_ClearScreen();
			InputSolution = 0;
			solved = 0;
			break;
		case FIRSTNUM:
			randVal = (rand()) % ((10 * difficulty) + 1);
			Solution = randVal;

			PrintText(num_to_str(randVal));
			break;
		case NUMBER:
			randVal = (rand()) % ((10 * difficulty) + 1);
			if (*operator == '+') {
				Solution = Solution + randVal;
			}	 
			else if (*operator == '-') {
				Solution = Solution - randVal;
				if (Solution < 0) {			// math problems should only be in the positives (negatives can't be inputted)
					Solution = Solution + randVal;	// revert previous math op
					displayColumn--;		// goes back to rewrite over previous '-'
					operator = "+";			// change math op to + instead
					PrintText(operator);
					Solution = Solution + randVal;	// performs add op
				}
			}
			
			PrintText(num_to_str(randVal));
			break;
		case OPERATOR:
			randVal = rand() % 2;
			if (randVal == 0) {
				operator = "+";
			}
			else if (randVal == 1) {
				operator = "-";
			}
			numOps++;

			PrintText(operator);
			break;
		case PRINT:
			PrintText("=\0");
			equationLen += displayColumn;	// displayColumn - 1 at this part is equal to the equation len
			break;
		case SOLVE:
			if (input != '\0') {
				PrintText(num_to_str(text_to_num(input)));
				InputSolution *= 10;	// shifts digit left
				InputSolution += text_to_num(input);	// adds new digit
				input = '\0'; 
			}
			failed = 0;
			break;
		case CHECK:
			input = '\0';	// Clears input; Transition to this state does not clear input. This is needed.
			if (Solution == InputSolution) {	// correct
				LCD_ClearScreen();
				displayColumn = 1;
				PrintText("YAY\0");
			}
			else {					// incorrect
				displayColumn = equationLen;
				LCD_Clean(displayColumn);
				InputSolution = 0;
				failed = 1;
			}
			break;  
	}
	// convert char return to number from getKeypad
		// nums => numbers
		// Reds => enter
		// Output as typing
	// use switches to create math, math is rand

	return state;	// return to do punishment
}

char* num_to_str(int number) {
	short numLen;	// length of number
	char* numTxt;
	char* termChar;
	short ones = number % 10;	// gets one's place
	short tens = number / 10;	// gets ten's place

	if (number < 10) {
		numLen = 1;
		numTxt = "0";
	}
	else {
		numLen = 2;
		numTxt = "00";
	}

	termChar = numTxt + numLen;
	*termChar = '\0';			// assigning term char at end of string
	if (tens == 0) {
		*(numTxt) = ones + '0';		// converts and stores ascii vers. of int
	}
	else {	
		*(numTxt) = tens + '0';		// converts and stores ascii vers. of int
		*(numTxt + 1) = ones + '0'; 	// converts and stores ascii vers. of int
	}

	return numTxt;
}

int text_to_num (unsigned char math) {
	switch (math) {
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case '0':
			return 0;
	}

	return 0;
}
void PrintText(char* text) {
	unsigned short textLen = sizeof(text) - 1;	// Gets Length of c-string - 1 (null term char)
	LCD_DisplayString(displayColumn, (const unsigned char *)(text));
	displayColumn += textLen;
}
void ComputeScore() {

}

enum SafeStates {LOCKED, ONE_UNLOCK, TWO_UNLOCK, ALARM};
int SafeSM(int state) {	// main sm, handles locked, unlocked, and in betweens (and fail)
	
	return state;	
}
