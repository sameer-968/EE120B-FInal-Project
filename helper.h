
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <util/delay.h>

#ifndef HELPER_H
#define HELPER_H

//Functionality - finds the greatest common divisor of two values
//Parameter: Two long int's to find their GCD
//Returns: GCD else 0
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a % b;
		if( c == 0 ) { return b; }
		a = b;
		b = c;
	}
	return 0;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

 
const unsigned int nums[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};
// a  b  c  d  e  f  g
void outNum(int num) {
    if (num < 0 || num > 9) return;

    PORTD &= ~(0b10111100);
    PORTC &= ~(0b00110000);

    unsigned int val = nums[num];

    if ((val & 32) != 0) {
        PORTD |= 0b00000100;
    }
    if ((val & 64) != 0) {
        PORTD |= 0b00001000;
    }
    if ((val & 16) != 0) {
        PORTD |= 0b00010000;
    }
    if ((val & 8) != 0) {
        PORTD |= 0b00100000;
    }
    if ((val & 4) != 0) {
        PORTD |= 0b10000000;
    }
    if ((val & 1) != 0) {
        PORTC |= 0b00100000;
    }
    if ((val & 2) != 0) {
        PORTC |= 0b00010000;
    }
}




//aFirst/Second: First range of values
//bFirst/Second: Range of values to map to
//inVal: value being mapped
unsigned int map_value(unsigned int aFirst, unsigned int aSecond, unsigned int bFirst, unsigned int bSecond, unsigned int inVal)
{
	return bFirst + (long((inVal - aFirst))*long((bSecond-bFirst)))/(aSecond - aFirst);
}

#endif /* HEPLER_H */