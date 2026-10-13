// Lab 1, task 4.
// PIC18F4520, UNI DS-6 board, mikroC PRO for PIC.
// Buttons RA0-RA5 -> strings to UART, UART -> LEDs RB0-RB7, both by interrupts.

#define FOSC 10000000UL
#define BAUD 19200UL

// BRG16 = BRGH = 1: baud = FOSC / (4 (n + 1)), n rounded to nearest.
// 19200 baud -> n = 129 (+0.16 %).
#define BRG_VALUE ((FOSC + 2 * BAUD) / (4 * BAUD) - 1)

// Pressed button level, set by the board's button jumper.
#define BTN_ACTIVE_HIGH 1
#define BTN_MASK 0x3F

#define MSG_COUNT 6
#define MSG_SIZE 20

// At least 7 characters each.
const char messages[MSG_COUNT][MSG_SIZE] = {
    "Button RA0\r\n", "Button RA1\r\n", "Button RA2\r\n",
    "Button RA3\r\n", "Button RA4\r\n", "Button RA5\r\n",
};

const char *tx_ptr;
volatile unsigned char tx_busy = 0;

void interrupt() {
    if (RCIE_bit && RCIF_bit) {
        LATB = RCREG;   // reading RCREG clears RCIF and FERR
        if (OERR_bit) { // overrun: receiver stalled until CREN is reset
            CREN_bit = 0;
            CREN_bit = 1;
        }
    }

    // TXIF can't be cleared in software, so TXIE is turned off at the end of the string.
    if (TXIE_bit && TXIF_bit) {
        if (*tx_ptr) {
            TXREG = *tx_ptr;
            tx_ptr++;
        } else {
            TXIE_bit = 0;
            tx_busy = 0;
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

// Returns 0 if the previous string is still being sent.
unsigned char uart_send_string(const char *str) {
    if (tx_busy)
        return 0;
    tx_ptr = str;
    tx_busy = 1;
    TXIE_bit = 1; // TXIF is already 1, the first interrupt fires immediately
    return 1;
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
            for (i = 0; i < MSG_COUNT; i++) {
                if (pressed & (1 << i)) {
                    while (tx_busy)
                        ;
                    uart_send_string(messages[i]);
                }
            }
            prev = cur;
        }
    }
}
