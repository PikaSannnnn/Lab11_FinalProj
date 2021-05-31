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

#define GREEN	0x02
#define RED	0x01	

unsigned char gameStartFlag = 1;// flag for if game has been started					// DEBUG
unsigned char difficulty = 1;	// 0 = none ; 1 = unsecure ; 2 = secure ; 3 = maximum security		// DEBUG
unsigned char maxOps = 1;	// maximum number of ops, initialized by difficulty			// DEBUG
unsigned char numCompleted = 0;	// resets at a lock unlock	// LED
unsigned char numUnlocks = 0;	// total number of unlocks, need 2 to open safe	// LED
unsigned char numAttempts = 0;	// total number of attempts, depends on difficulty
unsigned char timeAttempt = 0;	// total "time" value, multiplier to depends on difficulty	// LED
unsigned char failed = 0x00;	// whether or not problem is incorrectly solved, 1 => not correct
unsigned char input = 0x00;	// global keypad input
unsigned char timerLED = 0x00;	// flag for if timer LED should be lit/blinked
unsigned char displayColumn = 1;// column for LCD display
unsigned char endFlag = 0;	// flag for game end
const double frq = 0.00;	// frequency for speakers
int score = 0;			// score

// SM FUNCTIONS
int SetDifficultySM(int state);
int MathProblemSM(int state);
int SafeSM(int state);	// main sm, handles locked, unlocked, and in betweens (and fail)

// COMPUTATIONAL/OPERATIONS FUNCTIONS
int Input(int state);			// gets keypad input and sets to global input
char* num_to_str(int number);		// converts decimal number to char*
int text_to_num (unsigned char math);	// converts char* to decimal number
void PrintText(char* text);		// PrintsText (utilizes LCD_DisplayString)
unsigned char GetSuccessLED();		// Returns binary of which LEDs to light for "completed" LEDs
unsigned char GetUnlockLED();		// Returns binary of which LEDs to light for "unlocked" LEDs
void SetLights();			// Sets LED for register
void DisplaySeg(char* value);		// char* will be converted to char for sevenseg func
void ComputeScore();			// Computes score

int main(void) {
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;

	int randNum;
	int numPeriod = 0; 
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

	srand(0);					// DEBUG

	while (1) {
		if ((!gameStartFlag) && (!difficulty)) {	// game not started and difficulty not selected
			numPeriod++;
		}
		else if ((!gameStartFlag) && difficulty) {	// difficulty has been selected, seed rand
			randNum = (difficulty * numPeriod * 3) % 7;
			srand(randNum);
			gameStartFlag = 1;	// sets gameStartFlag to true, prevents this and above if from running
		}
		else if (gameStartFlag) {
			for (unsigned short i = 0; i < numTasks; i++) {
				if (tasks[i]->elapsedTime == tasks[i]->period) {
					tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
					tasks[i]->elapsedTime = 0;
				}
				tasks[i]->elapsedTime += 1;	
			}
			SetLights();
		}
		

		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}

enum DifficultyStates {EASY, MEDIUM, HARD, SELECT};
int SetDifficultySM(int state) {
	if (endFlag) {
		return state;
	}

	difficulty = 1;

	// outputs difficulties
	// sets selected difficulty
		// set numAttempts
		// set timeAttempt
	// difficulty chosen by cycling (and printing it out)
		// use A ^, C v, B select
	return state;
}
enum InputStates {WAIT, WAIT_RELEASE};
int Input(int state) {	// get input and store it -> handles single press only.
	if (endFlag) {
		return state;
	}
	
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
enum MathStates {MATH_CLEAR, FIRSTNUM, NUMBER, OPERATOR, PRINT, SOLVE, CHECK, UNLOCKING};
int MathProblemSM(int state) {	// prints and checks math inputs
	if (endFlag) {
		return state;
	}

	static unsigned char solved;	// whether or not problem is solved, 0 => not yet solved (but not failed)
	static unsigned char unlocking;	// unlocking flag
	static int Solution;		// solution to math
	static unsigned short equationLen;	// length of equation (used for displayColumn)
	static int InputSolution;	// solution to input
	static char* operator = "\0";	// opearator being used
	static short numOps;		// number of operators in equation (difficulty determines maximum # of operators)
	static int tmpVal;

	switch (state) {
		case MATH_CLEAR:
			state = FIRSTNUM;
			break;
		case FIRSTNUM:
			state = OPERATOR;
			break;
		case NUMBER:
			if (numOps < maxOps) {				// FIX maxOps UPDATE FOR DIFFICULTIES
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
			else if (solved && !unlocking) {
				state = MATH_CLEAR;
			}
			else if (solved && unlocking) {
				state = UNLOCKING;
			}
			break;
		case UNLOCKING:
			if (!unlocking) {
				state = MATH_CLEAR;
			}
			break;
		default:
			state = MATH_CLEAR;
			solved = 0;
			unlocking = 0;
			failed = 0;
			Solution = 0;
			InputSolution = 0;
			equationLen = 0;
			numCompleted = 0;
			numOps = 0;
			tmpVal = 0;
			break;
	}

	switch (state) {
		case MATH_CLEAR:
			LCD_ClearScreen();
			displayColumn = 1;
			solved = 0;
			unlocking = 0;
			failed = 0;
			Solution = 0;
			InputSolution = 0;
			equationLen = 0;
			numOps = 0;
			tmpVal = 0;
			break;
		case FIRSTNUM:
			tmpVal = (rand()) % (((15 + (5 * numUnlocks)) * difficulty) + 1);	// range between 0-10 inclusive, 0-15 inclusive if 1 is unlocked (for unsecure level), multiply by difficulty
			Solution = tmpVal;

			PrintText(num_to_str(tmpVal));
			break;
		case NUMBER:
			tmpVal = (rand()) % ((10 * difficulty) + 1);
			if (*operator == '+') {
				Solution = Solution + tmpVal;
			}	 
			else if (*operator == '-') {
				Solution = Solution - tmpVal;
				if (Solution < 0) {			// math problems should only be in the positives (negatives can't be inputted)
					Solution = Solution + tmpVal;	// revert previous math op
					displayColumn--;		// goes back to rewrite over previous '-'
					operator = "+";			// change math op to + instead
					PrintText(operator);
					Solution = Solution + tmpVal;	// performs add op
				}
			}
			
			PrintText(num_to_str(tmpVal));
			break;
		case OPERATOR:
			tmpVal = rand() % 2;
			if (tmpVal == 0) {
				operator = "+";
			}
			else if (tmpVal == 1) {
				operator = "-";
			}
			numOps++;

			PrintText(operator);
			break;
		case PRINT:
			PrintText("=\0");
			PrintText(num_to_str(Solution));	// DEBUG
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
				solved = 1;
				numCompleted++;
				if (numCompleted >= 3) {
					tmpVal = 0;
					unlocking = 1;
				}
			}
			else {					// incorrect
				displayColumn = equationLen;
				LCD_Clean(displayColumn);
				InputSolution = 0;
				failed = 1;
				numCompleted = 0;
				numAttempts--;
			}
			break;
		case UNLOCKING:
			if (tmpVal == 0) {
				displayColumn = 1;
				LCD_ClearScreen();
				PrintText("Unlocking");
			}
			else if (tmpVal == 500 || tmpVal == 1000 || tmpVal == 1500) {
				PrintText(".");
			}

			tmpVal++;
			if (tmpVal >= 2000) {	// 2 second unlock time
				unlocking = 0;
				numCompleted = 0;
				numUnlocks++;
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
enum SafeStates {PRE_GAME, GAME_INIT, LOCKED, ONE_UNLOCK, UNLOCKED, WAIT_END, END_GAME, ALARM};
int SafeSM(int state) {	// main sm, handles locked, unlocked, and in betweens (and fail)
	static int cnt;
	switch (state) {
		case PRE_GAME:
			if (gameStartFlag) {
				state = GAME_INIT;
			}
			break;
		case GAME_INIT:
			state = LOCKED;
			break;
		case LOCKED:
			if (numAttempts <= 0) {
				state = ALARM;
			}
			if (numUnlocks == 1) {
				state = ONE_UNLOCK;
			}
			break;
		case ONE_UNLOCK:
			if (numAttempts <= 0) {
				state = ALARM;
			}
			if (numUnlocks == 2) {
				state = UNLOCKED;
			}
			break;
		case UNLOCKED:
			state = WAIT_END;
			break;
		case WAIT_END:
			if (cnt >= 100) {	// Display End Message for 5 seconds
				state = END_GAME;
				cnt = 0;
			}
			break;
		case END_GAME:
			// Display Score
			break;
		case ALARM:			// ALARM will NOT include sound
			state = WAIT_END;
			break;
		default:
			state = PRE_GAME;
			cnt = 0;
			break;
	}

	switch (state) {
		case PRE_GAME:		// Resets important globals to default
			numCompleted = 0;
			numUnlocks = 0;	
			numAttempts = 0;
			timeAttempt = 0;
			failed = 0x00;
			input = 0x00;
			timerLED = 0;
			displayColumn = 1;
			endFlag = 0;
			score = 0;
			break;
		case GAME_INIT:
			maxOps = 1;
			endFlag = 0;
			numAttempts = 5;		// FIX FOR DIFFICULTIES, MEDIUM->3, HARD -> 1			// DEBUG			
			timeAttempt = 0;										// DEBUG
			break;
		case LOCKED:
			// nothing
			break;
		case ONE_UNLOCK:
			// nothing
			break;
		case UNLOCKED:
			endFlag = 1;
			LCD_ClearScreen();
			displayColumn = 1;
			PrintText("====  SAFE  ====");
			PrintText("==  UNLOCKED  ==");
			break;
		case WAIT_END:
			cnt++;
			break;
		case END_GAME:
			// Pressing any button should set gameStartFlag to false
			break;
		case ALARM:
			endFlag = 1;
			LCD_ClearScreen();
			displayColumn = 1;
			PrintText("===  ALARM  ===");
			PrintText("==  SOUNDED  ==");
			break;
	}
	return state;	
}


char* num_to_str(int number) {
	short ones = number % 10;	// gets one's place
	short tens = number / 10;	// gets ten's place

	if (tens == 0) {
		char* numTxt = "0";	
		*(numTxt) = (ones + '0');	// converts and stores ascii vers. of int
		return numTxt;
	}
	else {
		char* numTxt = "00";
		*(numTxt) = tens + '0';		// converts and stores ascii vers. of int
		*(numTxt + 1) = ones + '0'; 	// converts and stores ascii vers. of int
		return numTxt;
	}

	return "";
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
	// Calculate text length
	unsigned short textLen = 0;
	char* charPtr = text;
	while (*(charPtr) != '\0') {
		textLen++;	
		charPtr++;
	}

	LCD_DisplayString(displayColumn, (const unsigned char *)(text));
	displayColumn += textLen;
}
unsigned char GetSuccessLED() {
	if (numCompleted == 0) {
		return 0x00;
	}
	else if (numCompleted == 1) {
		return 0x01;
	}
	else if (numCompleted == 2) {
		return 0x03;
	}
	else if (numCompleted >= 3) {
		return 0x07;
	}

	return 0x00;
}
unsigned char GetUnlockLED() {
	unsigned char returnLEDs = (RED << 2) | RED;	// all red

	if (numUnlocks == 1) {
		returnLEDs = (RED << 2) | GREEN;
	}
	if (numUnlocks == 2) {
		returnLEDs = (GREEN << 2) | GREEN;
	}

	return returnLEDs;
}
void SetLights() {
	unsigned char ledOutput = 0x01;
	ledOutput = ledOutput << 3;
	ledOutput = ledOutput | GetSuccessLED();
	ledOutput = ledOutput << 4;
	ledOutput = ledOutput | GetUnlockLED();

	transmit_data(ledOutput);
}
void DisplaySeg(char* value) {
	char sevensegVal = *(value);	// number is guaranteed to be single digit (in first address of c-string)
	Write7Seg(sevensegVal);
}
void ComputeScore() {

}

