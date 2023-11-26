#ifndef COMMON_H
#define COMMON_H

#include "defs.h"

#define ASSERT(exp) ((exp) ? 1 : error_handler(__FILE__, __FUNCTION__, __LINE__, #exp))

void error_handler(const char *file, const char *function, int line, const char *exp);

void set_offset_yx(uint8_t height, uint8_t width, uint8_t *offset_y, uint8_t *offset_x);

#endif