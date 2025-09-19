
#include <avr/io.h>
#include "timerISR.h"
#include "LCD_init.h"
#include "display.h"
#include "spiAVR.h"
#include "periph.h"
#include "font.h"
#define NUM_TASKS 5
#define TURRET_WIDTH 20
#define TURRET_HEIGHT 10
#define BULLET_SIZE 6

typedef struct {
    char state;
    unsigned int period;
    unsigned int elapsedTime;
    int (*TickFct)(int);
} Task;

Task tasks[NUM_TASKS];

void setup_buzzer() {
    DDRD |= (1 << PD6);
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    TCCR0B = (TCCR0B & 0xF8) | 0x04;
}

void play_note(unsigned int freq) {
    unsigned int top = (F_CPU / (512 * freq)) - 1;
    OCR0A = top;
    TCCR0A |= (1 << COM0A1);
}

void stop_buzzer() {
    TCCR0A &= ~(1 << COM0A1);
    PORTD &= ~(1 << PD6);
}

const unsigned int sad_melody[10] = {262, 233, 220, 196, 175, 165, 147, 131, 123, 110};
const unsigned int happy_melody[6]= {523, 587, 659, 698, 784, 880};
const unsigned char sad_durations[10] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
const unsigned char happy_durations[6] = {2, 2, 2, 2, 1, 1};
unsigned char currentNote = 0;
unsigned int noteCounter = 0;
unsigned int TASK_1_PERIOD = 50;
unsigned int TASK_2_PERIOD = 300;
unsigned int TASK_3_PERIOD = 50;
unsigned int TASK_4_PERIOD = 50;
unsigned int TASK_5_PERIOD = 50;
unsigned int GCD_PERIOD = 50;
unsigned char turret_x = 60; 
unsigned char turret_y = 110;
unsigned char bullet_x = 0;
unsigned char bullet_y = 0;
unsigned char power_block_destroyed = 0;
unsigned char bullet_active = 0;
unsigned int fillScreen = 0;
unsigned char block_alive[2][4] = {
    {1, 1, 1, 1},
    {1, 1, 1, 1}
};
unsigned char gameOver = 0;
unsigned char y_offset = 0;
unsigned char turret_speed = 2;
unsigned char bullet_step = 4;
unsigned char allCleared;
unsigned char doneNote = 0;
unsigned char melody_enabled= 0;
unsigned char win = 0;

#define NORMAL_TURRET_SPEED 2
#define BOOSTED_TURRET_SPEED 4
#define NORMAL_BULLET_STEP 4
#define BOOSTED_BULLET_STEP 8

unsigned char boost_active = 0;
unsigned int boost_timer = 0;
const unsigned char block_x[4] = {10, 40, 70, 100};
const unsigned char block_y[2] = {10, 50};
const unsigned char block_size = 20;
unsigned int waveScore = 0;
enum MelodyStates { MELODY_START, PLAY_NOTE, WAIT_NOTE };
enum DisplayStates{ DISPLAY_START, RUNNING };
enum TurretStates { TURRET_START, TURRET_IDLE, TURRET_FIRE };
enum PowerupStates { POWERUP_WAIT, POWERUP_ACTIVE };
enum YouWinStates { WIN_WAIT, WIN_DISPLAYED };
void resetGame() {
    _delay_ms(600);
    tasks[2].period = 50;
    if(win || gameOver){
        waveScore = 0;
    }
    outNum(waveScore);
    bullet_active = 0;
    fill_screen_black();
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 4; col++) {
            block_alive[row][col] = 1;
        }
    }
    melody_enabled = 0;
    doneNote = 0;
    tasks[0].state = MELODY_START; 
    y_offset = 0;
    gameOver = 0;
    stop_buzzer();
    power_block_destroyed = 0;
    boost_active = 0;
    boost_timer = 0;
    win = 0;
    turret_speed = NORMAL_TURRET_SPEED;
    bullet_step = NORMAL_BULLET_STEP;
}

int MelodySM(int state) {
    const unsigned int* melody;
    const unsigned char* durations;
    switch (state) {
        case MELODY_START:
            stop_buzzer();
            if (gameOver == 1) {
                melody_enabled = 1;
                doneNote = 1;
            }
            else{
                melody_enabled = 0;
                stop_buzzer();
            }
            if (allCleared) {
                melody = happy_melody;
                durations = happy_durations;
                doneNote = 1;
                TCCR0B = (TCCR0B & 0xF8) | 0x03;
                waveScore++;
                outNum(waveScore);
            } else if (gameOver) {
                melody = happy_melody;
                durations = happy_durations;
                TCCR0B = (TCCR0B & 0xF8) | 0x04;
            }
            if (melody_enabled == 1 || doneNote == 1) {
                state = PLAY_NOTE;
            }
            break;
        case PLAY_NOTE:
            if ((currentNote < 6)) {
                play_note(melody[currentNote]);
                noteCounter = 0;
                state = WAIT_NOTE;
            } else {
                stop_buzzer();
                currentNote = 0;
                doneNote = 0;
                melody_enabled = 0;
                state = MELODY_START;
            }
            break;
        case WAIT_NOTE:
            if (noteCounter >= durations[currentNote] * 2) {
                currentNote++;
                state = PLAY_NOTE;
            } else {
                noteCounter++;
            }
            break;
        default:
            state = MELODY_START;
            break;
    }
    return state;
}

unsigned char displayInitialized = 0;
const unsigned char height = 20;
unsigned short RED = 0x001F;

int DisplaySM(int state){
    static unsigned char previous_y_offset = 0;
    const unsigned char height = 20;
    switch(state){
        case DISPLAY_START:
            state = RUNNING;
            break;
    }
    switch(state){
        case RUNNING:
         if (gameOver || win) return state; 
            if(!gameOver || !win){
                for (int row = 0; row < 2; row++) {
                    for (int col = 0; col < 4; col++) {
                        if (block_alive[row][col]) {
                            draw_rectangle(
                                block_x[col], 
                                block_y[row] + previous_y_offset, 
                                block_x[col] + block_size - 1, 
                                block_y[row] + previous_y_offset + height - 1, 
                                0x0000
                            );
                        }
                    }
                }
                for (int row = 0; row < 2; row++) {
                    for (int col = 0; col < 4; col++) {
                        if (block_alive[row][col]) {
                            unsigned int color;
                            if (row == 0 && col == 3) {
                                color = 0xFFE0;
                            } else {
                                color = 0x001F;
                            }
                            draw_rectangle(
                                block_x[col], 
                                block_y[row] + y_offset, 
                                block_x[col] + block_size - 1, 
                                block_y[row] + y_offset + height - 1, 
                                color
                            );
                        }
                    }
                }
                previous_y_offset = y_offset;
                y_offset++;
                if (y_offset > 50) {
                    y_offset = 0;
                }
            }
            break;
        default:
            break;
    }
    return state;
}

int TurretSM(int state) {
    static unsigned char prev_turret_x = 60;
    static unsigned char prev_bullet_y = 0;
    unsigned int x_input = ADC_read(1);

    if ((!(PINC & (1 << PC2)) && gameOver) || (!(PINC & (1 << PC2)) && win)) {
        resetGame();
        return TURRET_START;
    }

    if (!gameOver && !win) {
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 4; col++) {
                if (block_alive[row][col]) {
                    unsigned char blockBottom = block_y[row] + y_offset + block_size;
                    if (blockBottom >= turret_y) {
                        gameOver = 1;
                        fill_screen_black();
                        draw_text(60, 40, "YOU", 0x07E0);
                        draw_text(60, 60, "LOSE", 0x07E0);
                        tasks[2].period = 500;
                        return state;
                    }
                }
            }
        }
    }

    draw_rectangle(prev_turret_x, turret_y, prev_turret_x + TURRET_WIDTH - 1, turret_y + TURRET_HEIGHT - 1, 0x0000);

    switch (state) {
        case TURRET_START:
            state = TURRET_IDLE;
            break;

        case TURRET_IDLE:
            if ((PINC & (1 << PC3)) && !bullet_active) {
                bullet_active = 1;
                bullet_x = turret_x + TURRET_WIDTH / 2 - BULLET_SIZE / 2;
                bullet_y = turret_y - BULLET_SIZE;
                prev_bullet_y = bullet_y;
                state = TURRET_FIRE;
            }
            break;

        case TURRET_FIRE:
            if (bullet_y > 0) {
                bullet_y -= bullet_step;

                for (int row = 0; row < 2; row++) {
                    for (int col = 0; col < 4; col++) {
                        if (block_alive[row][col]) {
                            unsigned char bx = block_x[col];
                            unsigned char by = block_y[row] + y_offset;

                            if (bullet_x < bx + block_size &&
                                bullet_x + BULLET_SIZE > bx &&
                                bullet_y < by + block_size &&
                                bullet_y + BULLET_SIZE > by) {

                                int margin = 2;
                                unsigned char x0 = (bx > margin) ? bx - margin : 0;
                                unsigned char y0 = (by > margin) ? by - margin : 0;
                                unsigned char x1 = bx + block_size - 1 + margin;
                                unsigned char y1 = by + block_size - 1 + margin;
                                draw_rectangle(x0, y0, x1, y1, 0x0000);

                                block_alive[row][col] = 0;
                                bullet_active = 0;
                                state = TURRET_IDLE;
                            }
                        }
                    }
                }

                if (bullet_active) {
                    draw_rectangle(bullet_x, prev_bullet_y, bullet_x + BULLET_SIZE - 1, prev_bullet_y + BULLET_SIZE - 1, 0x0000);
                    draw_rectangle(bullet_x, bullet_y, bullet_x + BULLET_SIZE - 1, bullet_y + BULLET_SIZE - 1, 0xFFE0);
                    prev_bullet_y = bullet_y;
                }
            } else {
                draw_rectangle(bullet_x, bullet_y, bullet_x + BULLET_SIZE - 1, bullet_y + BULLET_SIZE - 1, 0x0000);
                bullet_active = 0;
                state = TURRET_IDLE;
            }
            break;

        default:
            state = TURRET_IDLE;
            break;
    }


    if (x_input < 400 && turret_x > 0) {
        turret_x -= turret_speed;
    } else if (x_input > 600 && turret_x < 128 - TURRET_WIDTH) {
        turret_x += turret_speed;
    }
    draw_rectangle(turret_x, turret_y, turret_x + TURRET_WIDTH - 1, turret_y + TURRET_HEIGHT - 1, 0xFFFF);
    prev_turret_x = turret_x;

    allCleared = 1;
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 4; col++) {
            if (block_alive[row][col]) {
                allCleared = 0;
                break;
            }
        }
        if (!allCleared) break;
    }

    if (allCleared && waveScore < 2) {
        resetGame();
        return TURRET_START;
    }

    return state;
}


int PowerupSM(int state) {
    int target_row = 0;
    int target_col = 3;
    switch (state) {
        case POWERUP_WAIT:
            if (!power_block_destroyed && block_alive[target_row][target_col] == 0) {
                power_block_destroyed = 1;
                boost_active = 1;
                boost_timer = 2000 / GCD_PERIOD;
                turret_speed = BOOSTED_TURRET_SPEED;
                bullet_step = BOOSTED_BULLET_STEP;
                state = POWERUP_ACTIVE;
            }
            break;
        case POWERUP_ACTIVE:
            if (boost_timer > 0) {
                boost_timer--;
            } else {
                boost_active = 0;
                turret_speed = NORMAL_TURRET_SPEED;
                bullet_step = NORMAL_BULLET_STEP;
                state = POWERUP_WAIT;
            }
            break;
        default:
            state = POWERUP_WAIT;
            break;
    }
    return state;
}
int YouWinSM(int state) {
    switch (state) {
        case WIN_WAIT:
            if ( waveScore == 2) {
                if(!win){
                fill_screen_black();
                }
                draw_text (60, 40,"YOU", 0x07E0);
                draw_text(60, 60, "WIN", 0x07E0); 
                win = 1;
                state = WIN_DISPLAYED;
            }
            break;

        case WIN_DISPLAYED:
            if(waveScore == 2){
                state = WIN_WAIT;
            }
            break;

        default:
            state = WIN_WAIT;
            break;
    }
    return state;
}
void TimerISR() {
    for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {
        if (tasks[i].elapsedTime == tasks[i].period){
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += GCD_PERIOD;
    }
}

int main(void) {
    
    DDRB |= (1 << PB3);
    DDRB |= (1 << PB5);
    DDRB |= (1 << PB2);
    DDRB |= (1 << PB1);
    DDRB |= (1 << PB0);
    DDRB |= (1 << PB4);
    DDRD |= (1 << PD6);
    DDRD = 0xFF;
    DDRC = 0xF0;
    DDRB = 0xFF;
    PORTB |= (1 << PB2);
    PORTC = 0x0F;
    PORTB |= (1 << PB4);
    while (PINC & (1 << PC3)); 
    while (!(PINC & (1 << PC3)));
    outNum(waveScore);
    ADC_init();
    setup_buzzer();
    SPI_INIT();
    ST7735_init();
    if(fillScreen == 0){
        fill_screen_black();
        fillScreen = 1;
    }

    PORTB |= (1 << PB4); 
    tasks[0].state = MELODY_START;
    tasks[0].period = TASK_1_PERIOD;
    tasks[0].elapsedTime = 0;
    tasks[0].TickFct = &MelodySM;
    tasks[1].state = 0;
    tasks[1].period = TASK_2_PERIOD;
    tasks[1].elapsedTime = 0;
    tasks[1].TickFct = &DisplaySM;
    tasks[2].state = TURRET_START;
    tasks[2].period = TASK_3_PERIOD;
    tasks[2].elapsedTime = 0;
    tasks[2].TickFct = &TurretSM;
    tasks[3].state = POWERUP_WAIT;
    tasks[3].period = TASK_4_PERIOD;
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &PowerupSM;
    tasks[4].state = WIN_WAIT;
    tasks[4].period = TASK_5_PERIOD;
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &YouWinSM;
    
    TimerSet(GCD_PERIOD);
    TimerOn();
    while (1) {}
    return 0;
}
