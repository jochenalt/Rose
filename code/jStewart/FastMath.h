#ifndef _FASTMATH_H_
#define _FASTMATH_H_

#include <avr/io.h>
#include <stdlib.h>

float sinusDegFast(float angle);
float cosinusDegFast(float angle);
float arcsinusRadFast(float x);
uint8_t sqrt_i16 (uint16_t q);
uint16_t sqrtFasti32 (uint32_t q);
float atan2Fast(float x, float y);


#define sign(x) (((x)>0.0)?1:(((x)<0)?-1:0))

int16_t average(int m, int16_t a[]);
int16_t median(int n, int16_t x[]);
int32_t varianz (int m, int16_t x[]);

#endif
