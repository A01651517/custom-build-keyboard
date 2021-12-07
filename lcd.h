/*
 * LCD Interfacing Library
 * Juan Alejandro Alcántara Minaya
 */

// Include utils for delays
#include <util/delay.h>

#ifndef AVR_IO_SHORCUTS_H
#define AVR_IO_SHORCUTS_H
#define high(x,y) x |= 1 << y
#define low(x,y) x &= ~(1 << y)
#define set(x,y,z) if (z) high(x,y); else low(x,y);
#endif /* AVR_IO_SHORCUTS_H */

/**
 * CONSTRAINTS
 */
#define LCD_MAX_CHARS 80
#define OFFSET 24
#ifndef LCD_COMMS_DELAY
#define LCD_COMMS_DELAY 20
#endif
unsigned int ROWS = 2;
unsigned int COLS = 40;

/**
 * GLOBALS
 */
unsigned int cursor_location = 0;

/**
 * LCD COMMANDS
 * https://mil.ufl.edu/3744/docs/lcdmanual/commands.html
 */
void set_command_mode() {
  low(LCD_RS_PORT, LCD_RS);
}
void set_write_mode() {
  high(LCD_RS_PORT, LCD_RS);
}
void open_comms() {
  high(LCD_E_PORT, LCD_E);
  _delay_us(1);
}
void close_comms() {
  // Close comms to LCD
  low(LCD_E_PORT, LCD_E);
  _delay_us(LCD_COMMS_DELAY);
}
void func_set_4bit_5x7 () {
  ROWS = 1;
  COLS = 40;
  set_command_mode();
  // Send MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  high(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
}
void func_set_4bit_5x7x2 () {
  ROWS = 2;
  COLS = 40;
  set_command_mode();
  // Send MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  high(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  high(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
}
void init_4bit_lcd (unsigned int rows) {
  _delay_ms(500); // Wait for power-on
  if (rows == 2) {
    func_set_4bit_5x7x2();
  } else {
    func_set_4bit_5x7();
  }
  _delay_ms(5); // Wait for function set
}
void clear_display() {
  set_command_mode();
  // Send MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  high(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
  _delay_ms(5);
}
void set_display(
    int display,
    int cursor,
    int blinking
) {
  set_command_mode();
  // Send MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  high(LCD_7_PORT, LCD_7);
  if (display)
    high(LCD_6_PORT, LCD_6);
  else
    low(LCD_6_PORT, LCD_6);
  if (cursor)
    high(LCD_5_PORT, LCD_5);
  else
    low(LCD_5_PORT, LCD_5);
  if (blinking)
    high(LCD_4_PORT, LCD_4);
  else
    low(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
}
void generate_char(char x) {
  set_write_mode();
  // Send MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  high(LCD_7_PORT, LCD_7);
  high(LCD_6_PORT, LCD_6);
  high(LCD_5_PORT, LCD_5);
  high(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
}

/**
 * Writing to the LCD
 * LCDs have a predefined charset that matches ASCII for the representation
 * of the characters in the screen. The display is able to store 80
 * characters, 40 per row in the 5 x 8 mode. The mysterious gap in memory
 * addressing was the result of a convenience consideration where the 7th bit
 * (the most significant) is used to determine in which row to store the
 * incoming information. This means that if you wanted to change the text to
 * the other row you only need to flip that 7th bit.
 *
 * What is stored in these addresses is a byte representing a character from
 * the predefined LCD character set:
 *
 * LCDs have a reserved space to store special characters called
 * the Character Generation RAM (CGRAM). It has 64 locations, however,
 * when in 5 x 8 mode, each character row needs one byte. This means that we
 * have 8 custom character locations of 8 byte each.
 * For instance:
 * Address 0x40 -> 1st character, 1st row
 * Address 0x41 -> 1st character, 2nd row
 * Address 0x42 -> 1st character, 3rd row
 * Address 0x43 -> 1st character, 4th row
 * Address 0x44 -> 1st character, 5th row
 * Address 0x45 -> 1st character, 6th row
 * Address 0x46 -> 1st character, 7th row
 * Address 0x47 -> 1st character, 8th row
 *
 * References:
 * http://web.alfredstate.edu/faculty/weimandn/lcd/lcd_addressing/lcd_addressing_index.html
 * https://mil.ufl.edu/3744/docs/lcdmanual/characterset.html
 * https://www.8051projects.net/lcd-interfacing/lcd-custom-character.php
 */
void store_char(unsigned char x) {
    set_write_mode();
    // Set the MSB nibble
    set(LCD_7_PORT, LCD_7, (x >> 7) & 0x01);
    set(LCD_6_PORT, LCD_6, (x >> 6) & 0x01);
    set(LCD_5_PORT, LCD_5, (x >> 5) & 0x01);
    set(LCD_4_PORT, LCD_4, (x >> 4) & 0x01);
    open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
    close_comms();
    // Send LSB nibble
    set(LCD_7_PORT, LCD_7, (x >> 3) & 0x01);
    set(LCD_6_PORT, LCD_6, (x >> 2) & 0x01);
    set(LCD_5_PORT, LCD_5, (x >> 1) & 0x01);
    set(LCD_4_PORT, LCD_4, x & 0x01);
    open_comms();
#endif
#endif
#endif
#endif
    close_comms();
    _delay_us(80);
}
void write_char(unsigned char x) {
  if (cursor_location < LCD_MAX_CHARS) {
    set_write_mode();
    // Set the MSB nibble
    set(LCD_7_PORT, LCD_7, (x >> 7) & 0x01);
    set(LCD_6_PORT, LCD_6, (x >> 6) & 0x01);
    set(LCD_5_PORT, LCD_5, (x >> 5) & 0x01);
    set(LCD_4_PORT, LCD_4, (x >> 4) & 0x01);
    open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
    close_comms();
    // Send LSB nibble
    set(LCD_7_PORT, LCD_7, (x >> 3) & 0x01);
    set(LCD_6_PORT, LCD_6, (x >> 2) & 0x01);
    set(LCD_5_PORT, LCD_5, (x >> 1) & 0x01);
    set(LCD_4_PORT, LCD_4, x & 0x01);
    open_comms();
#endif
#endif
#endif
#endif
    close_comms();
    cursor_location++;
  }
}
void move_cursor_left() {
  set_command_mode();
  // Set the MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  high(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
  _delay_us(120);
}
void move_cursor_right() {
  set_command_mode();
  // Set the MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  high(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  low(LCD_7_PORT, LCD_7);
  high(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
  _delay_us(120);

}
void go_to(unsigned int y, unsigned int x) {
  if (y < ROWS && x < COLS) {
    unsigned int location = y * COLS + x;
    if (location > cursor_location) {
      for (int i = 0; i < location - cursor_location; i++) {
        move_cursor_right();
      }
    } else {
      for (int i = 0; i < cursor_location - location; i++) {
        move_cursor_left();
      }
    }
    cursor_location = location + y * OFFSET;
  }
}
void go_home() {
  set_command_mode();
  // Set the MSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  low(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  low(LCD_7_PORT, LCD_7);
  low(LCD_6_PORT, LCD_6);
  high(LCD_5_PORT, LCD_5);
  low(LCD_4_PORT, LCD_4);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
  cursor_location = 0;
  _delay_ms(5);
}
void write_str(unsigned char* str) {
  unsigned char* ptr = str;
  while (*ptr) {
    write_char(*ptr);
    ptr++;
  }
}
void set_custom_char(unsigned int addr, unsigned int *character) {
  set_command_mode();
  // Set the MSB nibble
  low(LCD_7_PORT, LCD_7);
  high(LCD_6_PORT, LCD_6);
  set(LCD_5_PORT, LCD_5, (addr >> 5) & 0x01);
  set(LCD_4_PORT, LCD_4, (addr >> 4) & 0x01);
  open_comms();
#ifndef LCD_3
#ifndef LCD_2
#ifndef LCD_1
#ifndef LCD_0
  close_comms();
  // Send LSB nibble
  set(LCD_7_PORT, LCD_7, (addr >> 3) & 0x01);
  set(LCD_6_PORT, LCD_6, (addr >> 2) & 0x01);
  set(LCD_5_PORT, LCD_5, (addr >> 1) & 0x01);
  set(LCD_4_PORT, LCD_4, addr & 0x01);
  open_comms();
#endif
#endif
#endif
#endif
  close_comms();
  _delay_us(80);
  for (unsigned int i = 0; i < 8; i++) {
    store_char(character[i]);
  }
}
