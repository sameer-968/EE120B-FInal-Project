/*        Your Name & E-mail: Sameer Anjum sanju004@ucr.edu
*          Discussion Section: 023
*         Assignment: Final Project Demo 3
*     
*        
*         I acknowledge all content contained herein, excluding template or example code, is my own original work.
*
*         Demo Link: https://youtu.be/WFDuH-KZ5rk
*/
#ifndef LCD_INIT_H
#define LCD_INIT_H

#include <avr/io.h>
#include <util/delay.h>
#include "spiAVR.h"
#include "helper.h"

void Send_Command(unsigned int cmd) {
    PORTB = SetBit(PORTB, 2, 0);
    PORTB = SetBit(PORTB, 1, 0);
    SPI_SEND(cmd);
    PORTB = SetBit(PORTB, 2, 1);
}

void Send_Data(unsigned int data) {
    PORTB = SetBit(PORTB, 2, 0);
    PORTB = SetBit(PORTB, 1, 1);
    SPI_SEND(data);
    PORTB = SetBit(PORTB, 2, 1);
}

void HardwareReset() {
    PORTB = SetBit(PORTB, 0, 0);
    _delay_ms(200);
    PORTB = SetBit(PORTB, 0, 1);
    _delay_ms(200);
}

void ST7735_init() {
    HardwareReset();
    Send_Command(0x01);
    _delay_ms(150);

    Send_Command(0x11);
    _delay_ms(200);

    Send_Command(0x3A);
    Send_Data(0x05);
    _delay_ms(10);

    Send_Command(0x29);
    _delay_ms(200);
}

#endif
