/*
 * Counter
 * Peter González A01651517
 * Juan Alcantara A01703947
 * Royer Arenas A01209400
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
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <io.h>
#include "usbdrv.h"
#include "oddebug.h"
#include <usb.h>

#define LCD_COMMS_DELAY 40
#define LCD_E_PORT PORTC
#define LCD_RS_PORT PORTC
#define LCD_E PD1
#define LCD_RS PD0
#define LCD_4_PORT PORTC
#define LCD_5_PORT PORTC
#define LCD_6_PORT PORTC
#define LCD_7_PORT PORTC
#define LCD_4 PC2
#define LCD_5 PC3
#define LCD_6 PC4
#define LCD_7 PC5
#include "lcd.h"
#include "lcd_messages.h"

#define BTN_CONFIG_PIN PINB
#define BTN_CONFIG PB4

#define NORMAL 0
#define CONFIG 1

struct btnInfo {
    volatile unsigned char *pin;
    unsigned char btn;
};

#define BTNS_LEN 10
const struct btnInfo BTNS[BTNS_LEN] = {
    { .pin = &PINB, .btn = PB3 }, // Button 0
    { .pin = &PINB, .btn = PB2 }, // Button 1
    { .pin = &PINB, .btn = PB1 }, // Button 2
    { .pin = &PINB, .btn = PB0 }, // Button 3
    { .pin = &PIND, .btn = PD7 }, // Button 4
    { .pin = &PIND, .btn = PD6 }, // Button 5
    { .pin = &PIND, .btn = PD5 }, // Button 6
    { .pin = &PIND, .btn = PD4 }, // Button 7
    { .pin = &PIND, .btn = PD1 }, // Button 8
    { .pin = &PIND, .btn = PD0 }, // Button 9
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

static uchar reportBuffer[2]; /* HID Buffer */
static uchar idleRate; /* HID Idle Rate */
static uint pressedKey = 0;
static const uchar keys[NUM_KEYS + 1][2] = {
    {0, 0}, // Reserved (no event)
    {NO_MOD, KEY_1},
    {NO_MOD, KEY_2},
    {NO_MOD, KEY_3},
    {NO_MOD, KEY_4},
    {NO_MOD, KEY_5},
    {NO_MOD, KEY_6},
    {NO_MOD, KEY_7},
    {NO_MOD, KEY_8},
    {NO_MOD, KEY_9},
    {NO_MOD, KEY_0}
};
static void updateReportBuffer(uint key) {
    reportBuffer[0] = keys[key][0];
    reportBuffer[1] = keys[key][1];
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

/* Initial configuration for EEPROM */
uint8_t initFlag[1];
uint8_t eepromAux[1];
uint8_t kkeys[10]={'0','1','2','3','4','5','6','7','8','9'};
#define AVAILABLE_CHARS 36
static const char alpha[36] = {
    'a','b','c','d','e',
    'f','g','h','i','j',
    'k','l','m','n','o',
    'p','q','r','s','t',
    'u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9'
};
    
/* Function to turn on nano's integrated led */
void nano_led() {
    PORTB |= _BV(PB5);
    _delay_ms(500);
    PORTB &= ~(_BV(PB5));
    _delay_ms(500);
}

/* Function to search for the first button that is pressed.
 * It assumes that the grid is connected in order.
 * @returns de number of the first button that is pressed [0-9]. If there is
 * not a button pressed it returns -1; */
int poll_btns() {
    int btnID = 0;

    // Check buttons
    for (int i = 0; i < BTNS_LEN; i++) {
        wdt_reset();
        if (! ((*(BTNS[i].pin) >> BTNS[i].btn) & 1) ) {
            _delay_us(20);
            while (! ((*(BTNS[i].pin) >> BTNS[i].btn) & 1) ) wdt_reset();
            //nano_led();
            _delay_us(10);
            return btnID;
        }
        btnID++;
    }

    return -1;
}

/* Function to change value in eeprom for a certain key */
void eeprom_key_value(int pressedKey) {
    eeprom_read_block((void *) eepromAux,(const void * )2+pressedKey,1);
    //write_char(eepromAux[0]);
    //_delay_ms(4000);
    if ((int)eepromAux[0]>=97 && (int)eepromAux[0]<=122){
        eepromAux[0] -= 97;
    } else {
        eepromAux[0] = (int)eepromAux[0]-(48)+26;
    }
    //alphaIndex=currentConfig[4];
}

/* Function to init the PORTS for the buttons and LED,
 * and also init the LED screen */
static void init() {
    // BTN CONFIG, 0, R0 OUTPUT
    DDRB = (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4);
    // LED OUTPUT
    DDRC = (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);

    // BTN CONFIG, 0, R0 PULLUP
    PORTB = (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
    // BTN R1, R2 PULLUP
    PORTD = (1<<PD0) | (1<<PD1) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);

    //LCD setup
    init_4bit_lcd(2);
    set_display(1,0,0);
    go_home();
    clear_display();

    // Read EEPROM
    eeprom_read_block((void *) initFlag,(const void * )31,1);

    // Check if it is first time
    if((int)initFlag[0]==49) {
        eeprom_update_block((const void *) kkeys,(void * ) 2,11);
        initFlag[0]='1';
        eeprom_update_block((const void *) initFlag,(void * ) 31,1);
        write_str((unsigned char *)"      ok     ");
        wdt_reset();
        _delay_ms(1000);
        go_home();
        clear_display();
    }

    // Display intro message on LCD
    introMessage();

    wdt_reset();
    /*
    // Flash led to tell finish init
    for (int i = 0; i < 5; i++) {
        PORTB |= _BV(PB5);
        _delay_ms(50);
        PORTB &= ~(_BV(PB5));
        _delay_ms(50);
    }
    */
}

int main() {
    unsigned int mode = NORMAL;
    unsigned int alphaIndex = 0;
    // TODO: const
    char * nums[11]={"ZERO","ONE","TWO","THREE","FOUR","FIVE","SIX","SEVEN","EIGHT","NINE"};

    int btnPress, dirPress;
    // TODO: refactor this
    int flag;


    // Init Btns and LED screen
    init();

    // Turn on the watchdog timer with 2 seconds
    wdt_enable(WDTO_2S);

    odDebugInit();
    usbInit(); // Reserved V-USB procedure

    sei(); // Turn on interrupts
    DBG1(0x00, 0, 0);

    normalModeMessage();
    while(1) {  
        wdt_reset(); // Reset Watchdog timer to avoid reset
        usbPoll(); // Listen to host

        if(mode==NORMAL) {
            if (!(BTN_CONFIG_PIN >> BTN_CONFIG)) { // If the button is pressed
                while(!poll(BTN_CONFIG_PIN, BTN_CONFIG)) wdt_reset(); // Wait for its release
                configModeMessage();
                mode = CONFIG;
            }

            // TODO: change for pressedKey?
            btnPress = poll_btns();
            // TODO: send signal to host of the pressed key

        } else {
            flag = 0;
            while(poll(BTN_CONFIG_PIN, BTN_CONFIG)) {
                wdt_reset();
                if (!flag) {
                    // Wait until user has pressed a key
                    while( (btnPress = poll_btns()) == -1) wdt_reset();

                    // Read value from eeprom -> stored in eepromAux[0]
                    eeprom_key_value(btnPress);
                    alphaIndex=(int)eepromAux[0];
                    // Display current value in screen
                    updateCurrentConfig(nums[btnPress], (int)eepromAux[0], alpha);

                    flag = 1;
                }

                // Go right or go left
                dirPress = poll_btns();

                if (dirPress == 4) { // Button 4
                    alphaIndex = alphaIndex-1;
                    if (alphaIndex < 0) alphaIndex = 26;
                    // Update led display
                    updateCurrentConfig(nums[btnPress],alphaIndex,alpha);
                } else if (dirPress == 6) { // Button 6
                    alphaIndex = (alphaIndex+1)%AVAILABLE_CHARS;
                    // Update led display
                    updateCurrentConfig(nums[btnPress],alphaIndex,alpha);
                }
            }
            while(!poll(BTN_CONFIG_PIN, BTN_CONFIG)) wdt_reset(); // Wait for its release
            _delay_us(20); // Bounce-back delay

            // Write on eeprom
            kkeys[0] = alpha[alphaIndex];
            //kkeys[0]=(unsigned char)alphaIndex;
            eeprom_update_block((const void *) kkeys,(void * ) 2,11);

            // Return to normal mode
            clear_display();
            go_home();
            normalModeMessage();
            mode = NORMAL;
        }

        if (usbInterruptIsReady()) {
            updateReportBuffer(pressedKey);
            usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
        }
        pressedKey = 0;
    }
}
