/*        Your Name & E-mail: Sameer Anjum sanju004@ucr.edu
*          Discussion Section: 023
*         Assignment: Final Project Demo 3
*     
*        
*         I acknowledge all content contained herein, excluding template or example code, is my own original work.
*
*         Demo Link: https://youtu.be/WFDuH-KZ5rk
*/
#include <avr/io.h>
#include <util/delay.h>
#include "LCD_init.h"
#include "helper.h"


void fill_screen_black() {
    Send_Command(0x2A);
    Send_Data(0x00);
    Send_Data(0x00);
    Send_Data(0x00);
    Send_Data(0x7F);

    Send_Command(0x2B);
    Send_Data(0x00);
    Send_Data(0x00);
    Send_Data(0x00);
    Send_Data(0x7F);

    Send_Command(0x2C);

    for (unsigned int i = 0; i < (128 * 128); i++) {
        Send_Data(0x00);
        Send_Data(0x00);
    }
}

void draw_rectangle(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color) {
    Send_Command(0x2A);
    Send_Data(0x00);
    Send_Data(x0);
    Send_Data(0x00);
    Send_Data(x1);

    Send_Command(0x2B);
    Send_Data(0x00);
    Send_Data(y0);
    Send_Data(0x00);
    Send_Data(y1);

    Send_Command(0x2C);

    for (unsigned int i = 0; i < ((x1 - x0 + 1) * (y1 - y0 + 1)); i++) {
        Send_Data(color >> 8);
        Send_Data(color & 0xFF);
    }
}
