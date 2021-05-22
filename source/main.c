/* Author: Maverick Bautista
 * Lab Section: 023
 * Assignment: Lab 10  Exercise 1
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link: N/A 
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "keypad.h"
#include "bit.h"
#endif

//scheduler struct
typedef struct _task {

	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);

} task;

//GCD function
unsigned long int findGCD(unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while(1) {
		c = a%b;
		if(c==0) { return b; }
		a = b;
		b = c;
	}
	return 0;
}


//enum and states
enum Keypad_States { start, wait, on };

int keypadSM(int state) {
	unsigned char x = GetKeypadKey();
	switch(state) {
		case start:
			state = wait;
		break;

		case wait:
			if(x == '\0') {
				state = wait;
			}
			else {
				state = on;
			}
		break;

		case on:
			if(x == '\0') {
				state = wait;
			}
			else {
				state = on;
			}
		break;

		default:
			state = wait;
		break;
	}
	switch(state) {
		case start:
			PORTB = 0x00;
		break;

		case wait:
			PORTB = 0x00;
		break;

		case on:
			PORTB = 0x80;
		break;
			
		default:
		break;
	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
	static task task1;
	task *tasks[] = {&task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char start = -1;

	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &keypadSM;

	unsigned long GCD = tasks[0]->period;
	for(unsigned long i = 1; i < numTasks; i++) {
        	GCD = findGCD(GCD, tasks[i]->period);
	}

	TimerSet(GCD);
	TimerOn();	

    while (1) {
	for(unsigned long i = 0; i < numTasks; i++) {
		if(tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += GCD;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
