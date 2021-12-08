#include <avr/io.h>
#include <util/delay.h>

#define DELAY 500

int main() {
    DDRD = (1<<PD2) | (1<<PD3);
    while (1) {
        PORTD = PORTD & ~(_BV(PD2));
        PORTD = PORTD & ~(_BV(PD3));
        _delay_ms(DELAY);
        PORTD |= _BV(PD2);
        PORTD |= _BV(PD3);
        _delay_ms(DELAY);
    }
}
