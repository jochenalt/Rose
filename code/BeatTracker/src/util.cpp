/*
 * util.cpp
 *
 *  Created on: Feb 12, 2018
 *      Author: jochenalt
 */

#include "util.h"

#include <stdlib.h>
#include <chrono>
#include <unistd.h>
#include <iomanip>
#include <thread>

void delay_ms(long ms) {
	if (ms > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


uint32_t millis() {
	/*
    static uint32_t clockPerMs = (CLOCKS_PER_SEC)/1000;
    uint32_t c = clock();
    return c/clockPerMs;
    */
    static auto epoch = std::chrono::high_resolution_clock::from_time_t(0);
    auto now   = std::chrono::high_resolution_clock::now();
    auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - epoch).count();
    return mseconds;
}
