/*
 * I/O Interfacing library
 * Juan Alejandro Alcántara Minaya
 */
#include <avr/io.h>
#ifndef AVR_IO_SHORCUTS_H
#define AVR_IO_SHORCUTS_H
#define high(x,y) x |= 1 << y
#define low(x,y) x &= ~(1 << y)
#define set(x,y,z) if (z) high(x,y); else low(x,y);
#define poll(x,y) ((x >> y) & 1)
#endif /* AVR_IO_SHORCUTS_H */
