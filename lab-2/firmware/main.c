// Lab 2
// PIC18F4520, UNI DS-6 board, mikroC PRO for PIC.
// MCP3204 over SPI -> UART, one decimal code per line: "2048\r\n".

#define FOSC 10000000UL
#define BAUD 19200UL

// BRG16 = BRGH = 1: baud = FOSC / (4 (n + 1)), n rounded to nearest.
// 19200 baud -> n = 129 (+0.16 %).
#define BRG_VALUE ((FOSC + 2 * BAUD) / (4 * BAUD) - 1)

// Board line #CS1# (SW13.7). CHECK which MCU pin it reaches on the MCU card.
#define ADC_CS LATC0_bit
#define ADC_CS_TRIS TRISC0_bit

// CH0-CH1 on CN15, CH2-CH3 on CN16.
#define ADC_CHANNEL 0

void uart_init(void) {
    TRISC6_bit = 0;
    TRISC7_bit = 1;

    BAUDCON = 0x08; // BRG16 = 1
    SPBRGH = (BRG_VALUE >> 8) & 0xFF;
    SPBRG = BRG_VALUE & 0xFF;

    TXSTA = 0x24; // TX9 = 0, TXEN = 1, SYNC = 0, BRGH = 1
    RCSTA = 0x80; // SPEN = 1, receiver off
}

void uart_write(unsigned char byte) {
    while (!TXIF_bit)
        ;
    TXREG = byte;
}

void uart_write_value(unsigned int value) {
    unsigned char digits[5];
    unsigned char count = 0;

    do {
        digits[count++] = '0' + value % 10;
        value /= 10;
    } while (value);

    while (count)
        uart_write(digits[--count]);
    uart_write('\r');
    uart_write('\n');
}

void spi_init(void) {
    ADC_CS = 1;
    ADC_CS_TRIS = 0;

    TRISC3_bit = 0; // SCK
    TRISC4_bit = 1; // SDI
    TRISC5_bit = 0; // SDO

    // SPI mode 0,0.
    SSPSTAT = 0x40; // SMP = 0, CKE = 1
    SSPCON1 = 0x21; // SSPEN = 1, CKP = 0, Master, SCK = FOSC/16 (625 kHz at 10 MHz)
}

unsigned char spi_transfer(unsigned char byte) {
    SSPBUF = byte;
    while (!BF_bit)
        ;
    return SSPBUF; // reading SSPBUF clears BF
}

// Single-ended, 0-4095 (1 mV per code with the 4.096 V reference).
// Frame: 0000 0 1 SGL D2 | D1 D0 xx xxxx | xxxx xxxx.
unsigned int adc_read(unsigned char channel) {
    unsigned char high, low;

    ADC_CS = 0;
    spi_transfer(0x06);                // start bit, SGL/DIFF = 1, D2 = 0
    high = spi_transfer(channel << 6); // D1, D0; response: ? ? ? 0 B11 B10 B9 B8
    low = spi_transfer(0x00);          // response: B7-B0
    ADC_CS = 1;                        // CS edge resets the ADC bit counter

    return ((unsigned int)(high & 0x0F) << 8) | low;
}

// ~20 ms at 10 MHz.
void sample_delay(void) {
    unsigned int i;
    for (i = 0; i < 5000; i++)
        ;
}

void main() {
    ADCON1 = 0x0F; // AN pins digital
    CMCON = 0x07;  // comparators off

    uart_init();
    spi_init();

    while (1) {
        uart_write_value(adc_read(ADC_CHANNEL));
        sample_delay();
    }
}
