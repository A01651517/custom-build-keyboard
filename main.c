/*
 * Counter
 * Peter González A01651517
 * Juan Alcantara A01703947
 *
 * Diagram:
 *                   -----------
 *           PB5|D13 |   [ ]   | D12|PB4 -> BTN_CONFIG
 *               3v3 |         | D11|PB3 -> BTN_0
 *                   |    A    | D10|PB2 -> BTN_R00
 * LCD_RS <- PC0|A00 |    R    | D09|PB1 -> BTN_R01
 *  LCD_E <- PC1|A01 |    D    | D08|PB0 -> BTN_R02
 *  LCD_0 <- PC2|A02 |    U    | D07|PD7 -> BTN_R10
 *  LCD_1 <- PC3|A03 |    I    | D06|PD6 -> BTN_R11
 *  LCD_2 <- PC4|A04 |    N    | D05|PD5 -> BTN_R12
 *  LCD_3 <- PC5|A05 |    O    | D04|PD4 -> BTN_R20
 *           PC6|A06 |         | D03|PD3 -> USB_D-
 *           PC7|A07 |         | D02|PD2 -> USB_D+
 *                5V |         | GND
 *               RST |         | RST
 *               GND |         | RX0|PD0 -> BTN_R22
 *               VIN |         | TX1|PD1 -> BTN_R21
 *                   -----------
 *
 * Description:
 * Counter plus another animation.
 */

#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#define F_CPU 16000000 // 16 MhZ
#include <util/delay.h>

#include "usbdrv.h"
#include "io.h"

#define LCD_E_PORT PORTC
#define LCD_RS_PORT PORTC
#define LCD_E PC1
#define LCD_RS PC0
#define LCD_4_PORT PORTC
#define LCD_5_PORT PORTC
#define LCD_6_PORT PORTC
#define LCD_7_PORT PORTC
#define LCD_4 PC2
#define LCD_5 PC3
#define LCD_6 PC4
#define LCD_7 PC5
#include "lcd.h"

#define NUM_KEYS 10

#define BTN_CONFIG_PIN PINB
#define BTN_CONFIG PB4

#define USB_PORT PORTD
#define USB_DP PD2 // USB D+
#define USB_DM PD3 // USB D-

// Buttons that are on PORTB
#define BTNS_PINB_LEN 4
const char BTNS_PINB[BTNS_PINB_LEN] = {
    PB3, // Button 0
    PB2, // Button 1
    PB1, // Button 2
    PB0 // Button 3
};

// Buttons that are on PORTC
#define BTNS_PINC_LEN 0
const char BTNS_PINC[BTNS_PINC_LEN] = {};

// Buttons that are on PORTD
#define BTNS_PIND_LEN 0
const char BTNS_PIND[BTNS_PIND_LEN] = {
    PD7, // Button 4
    PD6, // Button 5
    PD5, // Button 6
    PD4, // Button 7
    PD1, // Button 8
    PD0 // Button 9
};

static uchar reportBuffer[2]; /* HID Buffer */
static uchar idleRate;

#define KEY_A       4
#define KEY_B       5
#define KEY_C       6
#define KEY_D       7
#define KEY_E       8
#define KEY_F       9
#define KEY_G       10
#define KEY_H       11
#define KEY_I       12
#define KEY_J       13
#define KEY_K       14
#define KEY_L       15
#define KEY_M       16
#define KEY_N       17
#define KEY_O       18
#define KEY_P       19
#define KEY_Q       20
#define KEY_R       21
#define KEY_S       22
#define KEY_T       23
#define KEY_U       24
#define KEY_V       25
#define KEY_W       26
#define KEY_X       27
#define KEY_Y       28
#define KEY_Z       29
#define KEY_1       30
#define KEY_2       31
#define KEY_3       32
#define KEY_4       33
#define KEY_5       34
#define KEY_6       35
#define KEY_7       36
#define KEY_8       37
#define KEY_9       38
#define KEY_0       39

#define KEY_UP      82
#define KEY_RIGHT   79
#define KEY_DOWN    81
#define KEY_LEFT    80

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

static const uchar keys[NUM_KEYS + 1][2] PROGMEM = {
    {0, 0},
    {0, KEY_1},
    {0, KEY_2},
    {0, KEY_3},
    {0, KEY_4},
    {0, KEY_5},
    {0, KEY_6},
    {0, KEY_7},
    {0, KEY_8},
    {0, KEY_9},
    {0, KEY_0},
}

const PROGMEM char usbHidReportDescriptor[35] = {   /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};
/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and we only allow one
 * simultaneous key press (except modifiers). We can therefore use short
 * 2 byte input reports.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */

/* Reserved function ran by the V-USB driver for the control USB endpoint */
uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t *request = (void *)data;
    usbMsgPtr = reportBuffer; // Reserved pointer to the transfer data
    if ((request->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        switch (request->bRequest) {
            case USBRQ_HID_GET_REPORT:
                // Send the pressed key to the computer
                return 1;
            case USBRQ_HID_GET_IDLE:
                // Send the delay to the computer
                usbMsgPtr = &idleRate;
                return 1;
            case USBRQ_HID_SET_IDLE:
                 // Get the default delay from the computer
                idleRate = request->wValue.bytes[1];
                break;
        }
    }
    return 0;
}

static void init() {
    // BTN CONFIG, 0, R0 OUTPUT
    DDRB = (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4);
    // LED OUTPUT
    DDRC = (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);

    // BTN CONFIG, 0, R0 PULLUP
    PORTB = (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
    // BTN R1, R2 PULLUP
    PORTD = (1<<PD0) | (1<<PD1) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);

    // Wait half a second for the microcontroller to reboot after reset
    for (uint i = 0; i < 250; i++) {wdt_reset(); _delay_ms(2);}
}

static void nano_led() {
    PORTB |= _BV(PB5);
    _delay_ms(500);
    PORTB &= ~(_BV(PB5));
    _delay_ms(500);
}


void config() { }

void print_btn(int i) { }

int main(int argc, char *argv[]) {
    init();

    for (int i = 0; i < 5; i++) {
        PORTB |= _BV(PB5);
        _delay_ms(100);
        PORTB &= ~(_BV(PB5));
        _delay_ms(100);
    }

    // Turn on the watchdog timer with 2 seconds
    wdt_enable(WDTO_2S);

    while(1) {
        wdt_reset();

        // Check Config Button
        if (!(BTN_CONFIG_PIN >> BTN_CONFIG)) {
            // TODO: Enter config mode
        }

        // Check buttons PORTB 
        for (int i = 0; i < BTNS_PINB_LEN; i++)
            if (! ((PINB >> BTNS_PINB[i]) & 1) )
                // TODO: Call button input
                nano_led();

        // Check buttons PORTC
        for (int i = 0; i < BTNS_PINC_LEN; i++)
            if (! ((PINC >> BTNS_PINC[i]) & 1) )
                // TODO: Call button input
                nano_led();

        // Check buttons PORTD
        for (int i = 0; i < BTNS_PIND_LEN; i++)
            if (! ((PINC >> BTNS_PIND[i]) & 1) )
                // TODO: Call button input
                nano_led();
    }
    return 0;
}
