#ifndef SPIAVR_H
#define SPIAVR_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Pin mapping based on your setup:
// SCK  = PB5 (Arduino D13)
// MOSI = PB3 (Arduino D11)
// SS   = PB2 (used as CS for ST7735)

#define PIN_SCK   PB5  // SPI Clock
#define PIN_MOSI  PB3  // Master Out Slave In
#define PIN_SS    PB2  // Chip Select (CS)

// Initializes SPI in Master Mode
void SPI_INIT() {
    // Set SCK, MOSI, and SS as outputs
    DDRB |= (1 << PIN_SCK) | (1 << PIN_MOSI) | (1 << PIN_SS);
    PORTB |= (1 << PB4); 
    // Enable SPI, Master mode, set clock rate fck/4
    SPCR |= (1 << SPE) | (1 << MSTR);
}

// Sends a single byte over SPI
void SPI_SEND(char data) {
    SPDR = data;                         // Start transmission
    while (!(SPSR & (1 << SPIF)));       // Wait for transmission complete
}

#endif /* SPIAVR_H */
