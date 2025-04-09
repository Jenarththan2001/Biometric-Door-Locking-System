/*
 * ds1307.h
 *
 *  Created on: Apr 4, 2025
 *      Author: jenai
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ds1307_init(void);
bool ds1307_get_time(uint8_t* hour, uint8_t* min, uint8_t* sec);
bool ds1307_set_time(uint8_t hour, uint8_t min, uint8_t sec);  

#ifdef __cplusplus
}
#endif
