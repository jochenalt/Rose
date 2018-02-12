/*
 * util.h
 *
 *  Created on: Feb 12, 2018
 *      Author: jochenalt
 */


#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <stdlib.h>
#include <iostream>


void delay_ms(long ms);
uint32_t millis();
char* getCmdOption(char ** begin, char ** end, const std::string & option);

#endif /* SRC_UTIL_H_ */
