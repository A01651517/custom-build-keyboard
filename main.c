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
#include <avr/io.h>
#include <util/delay.h>

#define LCD_E_PORT PORTC
#define LCD_RS_PORT PORTC
#define LCD_E PC1
#define LCD_RS PC0
#define LCD_0_PORT PORTC
#define LCD_1_PORT PORTC
#define LCD_2_PORT PORTC
#define LCD_3_PORT PORTC
#define LCD_0 PC2
#define LCD_1 PC3
#define LCD_2 PC4
#define LCD_3 PC5
#include "../libs/lcd.h"

#define BTN_CONFIG_PORT PORTB
#define BTN_CONFIG PB4

/*
#define BTN_0_PORT PORTB
#define BTN_0 PB3

#define BTN_R0_PORT PORTB
#define BTN_R00 PB2
#define BTN_R01 PB1
#define BTN_R02 PB0

#define BTN_R1_PORT PORTD
#define BTN_R10 PD7
#define BTN_R11 PD6
#define BTN_R12 PD5

#define BTN_R2_PORT PORTD
#define BTN_R20 PD4
#define BTN_R21 PD1
#define BTN_R22 PD0
*/

#define USB_PORT PORTD
#define USB_DP PD2 // USB D+
#define USB_DM PD3 // USB D-

#define BTNS_PORTS [ PORTB, PORTB, PORTB, PORTB, PORTD, PORTD, PORTD, PORTD, PORTD, PORTD ]
#define BTNS [PB3, PB2, PB1, PB0, PD7, PD6, PD5, PD4, PD1, PD0 ]

void config() { }

void print_btn(int i) { }

int main(int argc, char *argv[]) {
    // BTN CONFIG, 0, R0 OUTPUT
    DDRB = (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4);
    // LED OUTPUT
    DDRC = (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);
    // BTN R1, R2 OUTPUT
    DDRD = (1<<PD0) | (1<<PD1) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);

    while(1) {
        // Check Config Button
        if ( (BTN_CONFIG_PORT >> BTN_CONFIG) & 1) {
            // TODO: Enter config mode
        }

        // Check buttons
        for (int i = 0; i < 10; i++) {
            if ( (BTNS_PORT[i] >> BTNS[i]) & 1)
                // TODO: Call button input
                print_btn(i);
        }
    }
    return 0;
}
