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
 *  LCD_7 <- PC2|A02 |    U    | D07|PD7 -> BTN_R10
 *  LCD_6 <- PC3|A03 |    I    | D06|PD6 -> BTN_R11
 *  LCD_5 <- PC4|A04 |    N    | D05|PD5 -> BTN_R12
 *  LCD_4 <- PC5|A05 |    O    | D04|PD4 -> BTN_R20
 *           PC6|A06 |         | D03|PD3 -> USB_D-
 *           PC7|A07 |         | D02|PD2 -> USB_D+
 *                5V |         | GND
 *               RST |         | RST
 *               GND |         | RX0|PD0 -> BTN_R22
 *               VIN |         | TX1|PD1 -> BTN_R21
 *                   -----------
 *
 * Description:
 * USB Keypad with assignable keys HID
 *
 * References:
 * HIDKeys - An Example USB HID
 *  https://www.obdev.at/products/vusb/hidkeys.html
 */

#define uint unsigned int
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv.h"
#include "oddebug.h"
#include "io.h"

#define LCD_E_PORT PORTC
#define LCD_RS_PORT PORTC
#define LCD_E PC1
#define LCD_RS PC0
#define LCD_4_PORT PORTC
#define LCD_5_PORT PORTC
#define LCD_6_PORT PORTC
#define LCD_7_PORT PORTC
#define LCD_4 PC5
#define LCD_5 PC4
#define LCD_6 PC3
#define LCD_7 PC2
#include "lcd.h"

#define NUM_KEYS 10

#define BTN_CONFIG_PIN PINB
#define BTN_CONFIG PB4

#define USB_PORT PORTD
#define USB_DP PD2 // USB D+
#define USB_DM PD3 // USB D-

// Buttons that are on PORTB
#define BTNS_PINB_LEN 4
const uchar BTNS_PINB[BTNS_PINB_LEN] = {
    PB3, // Button 0
    PB2, // Button 1
    PB1, // Button 2
    PB0 // Button 3
};

// Buttons that are on PORTC
#define BTNS_PINC_LEN 0
const uchar BTNS_PINC[BTNS_PINC_LEN] = {};

// Buttons that are on PORTD
#define BTNS_PIND_LEN 6
const uchar BTNS_PIND[BTNS_PIND_LEN] = {
    PD7, // Button 4
    PD6, // Button 5
    PD5, // Button 6
    PD4, // Button 7
    PD1, // Button 8
    PD0 // Button 9
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
    0x81, 0x00,                    //   INPUT (Data,Array,Abs)
    0xc0                           // END_COLLECTION
};

/**
 * Official HID key values for keyboard
 * devices
 * https://usb.org/sites/default/files/hut1_2.pdf
 */
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
#define NO_MOD 0
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

static uchar reportBuffer[2]; /* HID Buffer */
static uchar idleRate; /* HID Idle Rate */
static uint pressedKey = 0;

static const uchar keys[NUM_KEYS + 1][2] = {
    {0, 0}, // Reserved (no event)
    {MOD_SHIFT_LEFT, KEY_A},
    {NO_MOD, KEY_2},
    {NO_MOD, KEY_3},
    {NO_MOD, KEY_4},
    {NO_MOD, KEY_5},
    {NO_MOD, KEY_6},
    {NO_MOD, KEY_7},
    {NO_MOD, KEY_8},
    {NO_MOD, KEY_9},
    {NO_MOD, KEY_0},
};

static void updateReportBuffer(uint key) {
    *reportBuffer = *keys[key];
}

/* Reserved function ran by the V-USB driver for the Control endpoint */
uchar usbFunctionSetup(uchar data[8]) {
    // We cast the data request
    usbRequest_t *request = (void *)data;
    // We make sure that the request is an HID class request
    if ((request->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        switch (request->bRequest) {
            // We send the last pressed key
            case USBRQ_HID_GET_REPORT:
                // We update the report buffer
                updateReportBuffer(pressedKey);
                // We set the reserved V-USB pointer to the report buffer
                usbMsgPtr = (short uint) reportBuffer;
                return sizeof(reportBuffer);
            // We share our idle rate with the host
            case USBRQ_HID_GET_IDLE:
                // Send the delay to the computer
                usbMsgPtr = (short uint) &idleRate;
                return 1;
            // We get the idle rate from the host
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

}

static void nano_led() {
    PORTB |= _BV(PB5);
    _delay_ms(500);
    PORTB &= ~(_BV(PB5));
    _delay_ms(500);
}

static void pollButtons (const uchar* btns, uint btns_len, uchar pin) {
    for (int i = 0; i < btns_len; i++) {
        if (!((pin >> btns[i]) & 1)) {
            pressedKey = 1;
            nano_led();
        }
    }
}

void config() { }

int main(int argc, char *argv[]) {
    uint lastKey = 0;

    // Turn on the watchdog timer with 2 seconds
    wdt_enable(WDTO_2S);
    init();
    odDebugInit();
    usbInit(); // Reserved V-USB procedure
    // Wait half a second for the microcontroller to reboot after reset
    for (uint i = 0; i < 250; i++) {wdt_reset(); _delay_ms(2);}
    sei(); // Turn on interrupts
    DBG1(0x00, 0, 0);

    for (int i = 0; i < 5; i++) {
        wdt_reset();
        PORTB |= _BV(PB5);
        _delay_ms(100);
        PORTB &= ~(_BV(PB5));
        _delay_ms(100);
    }


    while(1) {
        wdt_reset();
        usbPoll();

        // Check Config Button
        if (!(BTN_CONFIG_PIN >> BTN_CONFIG)) {
            // TODO: Enter config mode
        }

        pollButtons(BTNS_PINB, BTNS_PINB_LEN, PINB);
        pollButtons(BTNS_PINC, BTNS_PINC_LEN, PINC);
        pollButtons(BTNS_PIND, BTNS_PIND_LEN, PIND);

        if (lastKey != pressedKey) {
            lastKey = pressedKey;
        }

        if (usbInterruptIsReady()) {
            updateReportBuffer(lastKey);
            usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
        }
    }
    return 0;
}
