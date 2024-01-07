#ifndef SEG7_DISPLAY_H
#define SEG7_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

void initDisplay(void);
void fillDigits(uint8_t [], const uint8_t, int16_t, const bool);
void switchDigits(const uint8_t);
void switchSegments(const uint8_t [], const uint8_t, const bool);

#endif