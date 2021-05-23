/* Author: Maverick Bautista
 * Lab Section: 023
 * Assignment: Lab 10  Exercise 3
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link: https://youtu.be/B9lhNFWcJ4M
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

void set_PWM(double frequency) {	//frequency function
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0xFFFF; }

		else if (frequency > 31250) { OCR3A = 0x0000; }

		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);

	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

unsigned char tmpOff = 0x00;
unsigned char tmpOn = 0x00;

//enum and states
enum Keypad_States { start, w1, bar, w2, one, w3, two, w4, three, w5, four, w6, five, unlock };

int keypadSM(int state) {
	unsigned char x = GetKeypadKey();
	unsigned char butt = ~PINB & 0x80;
	switch(state) {
		case start:
			state = w1;
		break;

		case w1:
			if(x == '#') {
				state = bar;
			}
			else {
				state = w1;
			}
		break;

		case bar:
			if(x == '#') {
				state = bar;
			}
			else if(x == '\0') {
				state = w2;
			}
			else {
				state = w1;
			}
		break;

		case w2:
                        if(x == '1') {
                                state = one;
                        }
                        else {
                                state = w2;
                        }
                break;

                case one:
                        if(x == '1') {
                                state = one;
                        }
                        else if(x == '\0') {
                                state = w3;
                        }
                        else {
                                state = w1;
                        }
                break;

		case w3:
                        if(x == '2') {
                                state = two;
                        }
                        else {
                                state = w3;
                        }
                break;

                case two:
                        if(x == '2') {
                                state = two;
                        }
                        else if(x == '\0') {
                                state = w4;
                        }
                        else {
                                state = w1;
                        }
                break;

		case w4:
                        if(x == '3') {
                                state = three;
                        }
                        else {
                                state = w4;
                        }
                break;

                case three:
                        if(x == '3') {
                                state = three;
                        }
                        else if(x == '\0') {
                                state = w5;
                        }
                        else {
                                state = w1;
                        }
                break;

		case w5:
                        if(x == '4') {
                                state = four;
                        }
                        else {
                                state = w5;
                        }
                break;

                case four:
                        if(x == '4') {
                                state = four;
                        }
                        else if(x == '\0') {
                                state = w6;
                        }
                        else {
                                state = w1;
                        }
                break;

		case w6:
                        if(x == '5') {
                                state = five;
                        }
                        else {
                                state = w6;
                        }
                break;

                case five:
                        if(x == '5') {
                                state = five;
                        }
                        else if(x == '\0') {
                                state = unlock;
                        }
                        else {
                                state = w1;
                        }
                break;

		case unlock:
			if(butt) {
				state = start;
			}
			else {
				state = unlock;
			}
		break;

		default:
			state = w1;
		break;
	}
	switch(state) {
		case unlock:
			tmpOn = 0x01;
		break;
	}
	return state;
}

enum Lock_States { pause, lock };

int lockSM(int state) {
	unsigned char tmpA = ~PINB & 0x80;
	switch(state) {
		case pause:
			if(tmpA) {
				state = lock;
			}
			else {
				state = pause;
			}
		break;

		case lock:
			if(tmpA) {
				state = lock;
			}
		break;

		default:
			state = pause;
		break;
	}

	switch(state) {
		case pause:
			tmpOff = 0x01;
		break;

		case lock:
			tmpOff = 0x00;
		break;

		default:
		break;
	}
	return state;
}


enum doorbell_States { initial, wait, play, done };
double notes[6] = {440.00, 261.63, 440.00, 392.00, 440.00, 261.63};
int time[6] = {2, 3, 5, 10, 11, 15};
unsigned char i = 0;
unsigned char h = 0;
unsigned char cnt = 0;
int doorbellSM(int state) {
        switch(state) {
                case initial:
                    state = wait;
                break;

                case wait:
		    i = 0;
                    h = 0;
                    cnt = 0;
		    if((~PINA & 0x80) == 0x80) {
                        state = play;
                    }
                break;

                case play:
                    if(i == 6) {
                        state = done;
		    }
                    else {
                        state = play;
		    }
                    if(cnt == time[h]) {
                        i++;
                        h++;
                    }
                    cnt++;
                break;

                case done:
                    if((~PINA & 0x80) == 0x00) {
                        state = wait;
		    }
                break;

                default:
                    state = initial;
                break;
	}

	switch(state) {
		case initial:
			set_PWM(0);
		break;

		case wait:
			set_PWM(0);
		break;

		case play:
			set_PWM(notes[i]);
		break;

		case done:
			set_PWM(0);
		break;
	}
        return state;
}

enum combine_States { combine };

int combineSM(int state) {
	switch(state) {
		case combine:
		        PORTB = tmpOn & tmpOff;
		break;
	}
	return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
	static task task1, task2, task3, task4 ;
	task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char start = -1;

	task1.state = start;
	task1.period = 200;
	task1.elapsedTime = task1.period;
	task1.TickFct = &keypadSM;

	task2.state = pause;
	task2.period = 200;
	task2.elapsedTime = task2.period;
	task2.TickFct = &lockSM;

	task3.state = combine;
	task3.period = 200;
	task3.elapsedTime = task3.period;
	task3.TickFct = &combineSM;

	task4.state = initial;
        task4.period = 200;
        task4.elapsedTime = task4.period;
        task4.TickFct = &doorbellSM;


	unsigned long GCD = tasks[0]->period;
	for(unsigned long i = 1; i < numTasks; i++) {
        	GCD = findGCD(GCD, tasks[i]->period);
	}

	PWM_on();
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
