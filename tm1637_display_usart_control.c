// Set CPU clock speed for accurate delays
#define F_CPU 16000000UL

#include <avr/io.h>      // AVR register definitions
#include <util/delay.h>  // Delay functions like _delay_us()
#include <avr/interrupt.h> // For interrupt support

// Define port and pin mappings for CLK and DIO lines
#define CLK_PORT PORTF
#define CLK_DDR  DDRF
#define CLK_PIN  PF1     // CLK connected to PF1 (Analog pin A1)

#define DIO_PORT PORTF
#define DIO_DDR  DDRF
#define DIO_PIN  PF0     // DIO connected to PF0 (Analog pin A0)


//Volatile: tells the compiler not to optimize anything that has to do with the volatile variable.
volatile char buffer[5];
volatile uint8_t count = 0;
volatile uint8_t numberReady = 0;

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

void initUsart(){ //Usart setup
    UCSR0A = 0;  // Control/Status Register A: default value
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);  // Enable Receiver and Transmitter and RX interrupt
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set frame: 8 data bits, 1 stop bit
    UBRR0H = 0;             // High byte of baud rate register
    UBRR0L = 8;           // Low byte of baud rate register for 115200 baud (with 16 MHz clock)
}

char readChar(){
    while (!(UCSR0A & (1 << RXC0))); // Wait until receive complete flag is set
    return UDR0;                     // Return the received character from the buffer
}

void writeChar(char x) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait until transmit buffer is empty
    UDR0 = x;                         // Send character
}

void writeString(char st[]) {
    for(uint8_t i = 0; st[i] != 0; i++) {
        writeChar(st[i]);             // Send each character one-by-one
    }
}

ISR(USART0_RX_vect) { //Usart interrupt for receiving
    char c = UDR0;

    if (c >= '0' && c <= '9' && count < 4) {
        buffer[count++] = c; //Fill the buffer with data received from usart
        writeChar(c); // Echo

        if (count == 4) {
            buffer[4] = '\0'; // Null-terminate
            numberReady = 1;
        }
    }
}

//
// SEGMENT MAPPING FOR DIGITS
// Each byte represents a 7-segment pattern (bits: a–g + dp)
//

const uint8_t SEGMENTS[] = { //SEGMENTS[] is an array of 8-bit values (uint8_t).
    //DIGITS
    0b00111111, // Number: 0 at index 0
    0b00000110, // Number: 1 at index 1
    0b01011011, // Number: 2 at index 2
    0b01001111, // Number: 3 at index 3
    0b01100110, // Number: 4 at index 4
    0b01101101, // Number: 5 at index 5
    0b01111101, // Number: 6 at index 6
    0b00000111, // Number: 7 at index 7
    0b01111111, // Number: 8 at index 8
    0b01101111,  // Number: 9 at index 9
};

//
// DISPLAY FUNCTION
// Writes 4 digits to the display
//

void tm_displayDigits(uint16_t num) {
    // Extract digits from a number
    uint8_t d0 = (num / 1000) % 10;
    uint8_t d1 = (num / 100) % 10;
    uint8_t d2 = (num / 10) % 10;
    uint8_t d3 = num % 10;

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

    initUsart();

    sei();  // Enable global interrupts

    while (1) {
        if (numberReady) {
            uint16_t number = 0;

            // Convert ASCII chars ('0'–'9') to integer digits:
            // '0' in ASCII is 48, so subtracting '0' (or 48) gives the actual digit value.
            // Example: '3' - '0' = 51 - 48 = 3
            for (uint8_t i = 0; i < 4; i++) {
                // Build full number by shifting left (×10) and adding each new digit.
                number = number * 10 + (buffer[i] - '0');
            }

            writeString("\r\nDisplaying number...\r\n");
            tm_displayDigits(number);

            // Reset for next input
            count = 0;
            numberReady = 0;
            writeString("Give a 4-digit number: ");
        }
    }
}

/*
 * - - a --
 * |       |
 * f       b
 * |       |
 * -- g --
 * |       |
 * e       c
 * |       |
 * -- d --   (and sometimes dp = decimal point)
 *
 * Bits are mapped as:
 * Bit:    0  1  2  3  4  5  6  7
 * Seg:    dp a  b  c  d  e  f  g   */
