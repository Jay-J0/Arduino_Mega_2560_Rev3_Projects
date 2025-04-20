/*This code controls a TM1637 4-digit 7-segment display using bit-banging to communicate with the module.
 * The code is designed to send data to the display to show numbers.
 * It utilizes two I/O pins (for Clock (CLK) and Data (DIO)) on an AVR microcontroller (such as the ATmega2560).
 * It manually handles the start/stop conditions and data transmission, as well as the timing for each operation.
 *
 * The tm_start and tm_stop functions initiate and terminate communication with the TM1637.
 *
 * tm_writeByte sends a byte of data to the TM1637.
 *
 * The SEGMENTS array stores the binary patterns for each digit (0-9) to be displayed.
 *
 * The tm_displayDigits function sends the digits to the display, setting it to auto-increment mode and turning on the display with brightness level 7.
 *
 * The main loop continuously displays the numbers "1234" for 1 second, then "8888" for 1 second, and repeats.*/

// Set CPU clock speed for accurate delays
#define F_CPU 16000000UL

#include <avr/io.h>      // AVR register definitions
#include <util/delay.h>  // Delay functions like _delay_us()

// Define port and pin mappings for CLK and DIO lines
#define CLK_PORT PORTF
#define CLK_DDR  DDRF
#define CLK_PIN  PF1     // CLK connected to PF1 (Analog pin A1)

#define DIO_PORT PORTF
#define DIO_DDR  DDRF
#define DIO_PIN  PF0     // DIO connected to PF0 (Analog pin A0)

//
// GPIO CONTROL FUNCTIONS
//

// Set CLK high (open-drain style): configure pin as input (hi-Z with pull-up)
void clk_high() {
    CLK_DDR &= ~(1 << CLK_PIN);  // Set DDR bit to 0 = input mode
}

// Set CLK low: configure pin as output and drive it low
void clk_low() {
    CLK_DDR |= (1 << CLK_PIN);   // Set DDR bit to 1 = output mode
    CLK_PORT &= ~(1 << CLK_PIN); // Set output to LOW
}

// Set DIO high (release line): input mode (open-drain style)
void dio_high() {
    DIO_DDR &= ~(1 << DIO_PIN);  // Set as input (line floats high via pull-up)
}

// Set DIO low: output mode and drive low
void dio_low() {
    DIO_DDR |= (1 << DIO_PIN);   // Set as output
    DIO_PORT &= ~(1 << DIO_PIN); // Set output to LOW
}

//
// TM1637 PROTOCOL CONTROL
//

// Send start condition to TM1637
void tm_start() {
    dio_high();   // DIO high
    clk_high();   // CLK high
    _delay_us(10);
    dio_low();    // DIO goes low while CLK is high = START
    _delay_us(10);
    clk_low();    // Begin data transmission
}

// Send stop condition to TM1637
void tm_stop() {
    clk_low();    // CLK low
    _delay_us(10);
    dio_low();    // DIO low
    _delay_us(10);
    clk_high();   // CLK high
    _delay_us(10);
    dio_high();   // DIO goes high while CLK is high = STOP
}

// Send a single byte to TM1637 (LSB first)
void tm_writeByte(uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) { //The byte is 8 bits long. The loop runs 8 times, one for each bit in the byte.
        clk_low();                     // CLK low before setting bit
        if (b & 0x01) dio_high();      // Send LSB
        else          dio_low();
        _delay_us(10);
        clk_high();                    // Clock HIGH to latch bit
        _delay_us(10);
        b >>= 1;                       // Shift to next bit
    }

    // Handle ACK bit from TM1637 (we ignore the actual value here)
    clk_low();
    dio_high(); // Release DIO (input mode)
    _delay_us(10);
    clk_high(); // Slave pulls line low for ACK (ignored here)
    _delay_us(10);
    clk_low();
}

//
// SEGMENT MAPPING FOR DIGITS
// Each byte represents a 7-segment pattern (bits: a–g + dp)
//

const uint8_t SEGMENTS[] = {
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

//
// DISPLAY FUNCTION
// Writes 4 digits to the display
//

void tm_displayDigits(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
    // Command 1: set data write mode (0x40 = auto-increment address mode)
    tm_start();
    tm_writeByte(0x40);
    tm_stop();

    // Command 2: set starting address (0xC0 = address 0)
    tm_start();
    tm_writeByte(0xC0);              // Start writing at digit 0
    tm_writeByte(SEGMENTS[d0]);     // Digit 0
    tm_writeByte(SEGMENTS[d1]);     // Digit 1
    tm_writeByte(SEGMENTS[d2]);     // Digit 2
    tm_writeByte(SEGMENTS[d3]);     // Digit 3
    tm_stop();

    // Command 3: control display (0x88 = display ON + brightness 7)
    tm_start();
    tm_writeByte(0x88 | 0x07);      // Turn on display, brightness max
    tm_stop();
}

//
// MAIN LOOP
//

int main(void) {
    // Ensure DIO and CLK pins are available (set initially as outputs)
    CLK_DDR |= (1 << CLK_PIN);
    DIO_DDR |= (1 << DIO_PIN);

    while (1) {
        tm_displayDigits(1, 2, 3, 4);  // Display "1234"
        _delay_ms(1000);              // Wait 1 second

        tm_displayDigits(8, 8, 8, 8);  // Display "8888"
        _delay_ms(1000);              // Wait 1 second
    }

    return 0;
}
