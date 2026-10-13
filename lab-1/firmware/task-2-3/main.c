// Lab 1, tasks 2-3
// PIC18F4520, UNI DS-6 board, mikroC PRO for PIC.

#define FOSC 10000000UL
#define BAUD 19200UL

// BRG16 = BRGH = 1: baud = FOSC / (4 (n + 1)), n rounded to nearest.
// 19200 baud -> n = 129 (+0.16 %).
#define BRG_VALUE ((FOSC + 2 * BAUD) / (4 * BAUD) - 1)

// Pressed button level, set by the board's button jumper.
#define BTN_ACTIVE_HIGH 1
#define BTN_MASK 0x3F

void interrupt() {
    if (RCIE_bit && RCIF_bit) {
        LATB = RCREG;   // reading RCREG clears RCIF and FERR
        if (OERR_bit) { // overrun: receiver stalled until CREN is reset
            CREN_bit = 0;
            CREN_bit = 1;
        }
    }
}

void uart_init(void) {
    TRISC6_bit = 0;
    TRISC7_bit = 1;

    BAUDCON = 0x08; // BRG16 = 1
    SPBRGH = (BRG_VALUE >> 8) & 0xFF;
    SPBRG = BRG_VALUE & 0xFF;

    TXSTA = 0x24; // TX9 = 0, TXEN = 1, SYNC = 0, BRGH = 1
    RCSTA = 0x90; // SPEN = 1, RX9 = 0, CREN = 1
}

void uart_write(unsigned char byte) {
    while (!TXIF_bit)
        ;
    TXREG = byte;
}

// ~20 ms at 10 MHz.
void debounce_delay(void) {
    unsigned int i;
    for (i = 0; i < 5000; i++)
        ;
}

unsigned char read_buttons(void) {
    unsigned char state = PORTA & BTN_MASK;
#if !BTN_ACTIVE_HIGH
    state ^= BTN_MASK;
#endif
    return state;
}

void main() {
    unsigned char prev = 0;
    unsigned char cur, pressed, i;

    ADCON1 = 0x0F; // AN pins digital
    CMCON = 0x07;  // comparators off
    TRISA |= BTN_MASK;
    LATB = 0;
    TRISB = 0;

    uart_init();

    RCIE_bit = 1;
    PEIE_bit = 1;
    GIE_bit = 1;

    while (1) {
        cur = read_buttons();
        if (cur != prev) {
            debounce_delay();
            if (read_buttons() != cur)
                continue;

            pressed = cur & ~prev;
            for (i = 0; i < 6; i++) {
                if (pressed & (1 << i))
                    uart_write('0' + i);
            }
            prev = cur;
        }
    }
}
