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

#define F_CPU 16000000UL  // 16 MHz

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "io.h"

#define LCD_COMMS_DELAY 40
#define LCD_E_PORT PORTC
#define LCD_RS_PORT PORTC
#define LCD_E PD1
#define LCD_RS PD0
#define LCD_4_PORT PORTC
#define LCD_5_PORT PORTC
#define LCD_6_PORT PORTC
#define LCD_7_PORT PORTC
#define LCD_4 PD2
#define LCD_5 PD3
#define LCD_6 PD4
#define LCD_7 PD5
#include "lcd.h"
#include "lcd_messages.h"

#define BTN_CONFIG_PIN PINB
#define BTN_CONFIG PB4

#define USB_PORT PORTD
#define USB_DP PD2 // USB D+
#define USB_DM PD3 // USB D-

#define NORMAL 0
#define CONFIG 1

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

void config() { }

void nano_led() {
    PORTB |= _BV(PB5);
    _delay_ms(500);
    PORTB &= ~(_BV(PB5));
    _delay_ms(500);
}

unsigned int mode = NORMAL;
unsigned int alphaIndex = 0;
char  currentKey[10];
char alpha[37]={'a','b','c','d','e',
                'f','g','h','i','j',
                'k','l','m','n','o',
                'p','q','r','s','t',
                'u','v','w','x','y','z',
                '1','2','3','4','5','6','7','8','9','0'};
char * nums[11]={"ZERO","ONE","TWO","THREE","FOUR","FIVE","SIX","SEVEN","EIGHT","NINE"};

int main(int argc, char *argv[]) {
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
    introMessage();
    for (int i = 0; i < 5; i++) {
        PORTB |= _BV(PB5);
        _delay_ms(100);
        PORTB &= ~(_BV(PB5));
        _delay_ms(100);
    }

    while(1) {  
        if(mode==NORMAL){
            normalModeMessage();
            // Check Config Button
            if (!(BTN_CONFIG_PIN >> BTN_CONFIG)) {
                // TODO: Enter config mode
            }

            if (!poll(BTN_CONFIG_PIN, BTN_CONFIG)) { // If the button is pressed
                _delay_us(20); // Bounce-back delay
                while(!poll(BTN_CONFIG_PIN, BTN_CONFIG)); // Wait for its release
                _delay_us(20); // Bounce-back delay
                configModeMessage();
                mode=CONFIG;
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
        }else{
            while(poll(BTN_CONFIG_PIN, BTN_CONFIG)){
                if (!poll(PINB, PINB3)) { // If the button is pressed
                    _delay_us(20); // Bounce-back delay
                    while(!poll(PINB, PINB3)); // Wait for its release
                    _delay_us(20); // Bounce-back delay
                    updateCurrentConfig(nums[4],alphaIndex,alpha);
                }
                
                if (!poll(PINB, PINB2)) { // If the button is pressed
                    _delay_us(20); // Bounce-back delay
                    while(!poll(PINB, PINB2)); // Wait for its release
                    _delay_us(20); // Bounce-back delay
                    //check bouds
                    alphaIndex++;
                    updateCurrentConfig(nums[4],alphaIndex,alpha);
                }
            }
            while(!poll(BTN_CONFIG_PIN, BTN_CONFIG)); // Wait for its release
            _delay_us(20); // Bounce-back delay
            normalModeMessage();
            mode=NORMAL;
        }
    }
    return 0;
}
