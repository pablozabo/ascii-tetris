#ifndef SHAPES_H
#define SHAPES_H

#include "defs.h"

const bool  c_shape_i[] = {
	1, 0, 0, 0, 
	1, 0, 0, 0, 
	1, 0, 0, 0, 
    1, 0, 0, 0
};

const bool c_shape_o[] = {
	0, 0, 0, 0, 
	0, 1, 1, 0, 
	0, 1, 1, 0, 
    0, 0, 0, 0
};

const bool c_shape_t[] = {
	0, 1, 0, 0, 
	1, 1, 1, 0, 
	0, 0, 0, 0, 
    0, 0, 0, 0
};

const bool c_shape_j[] = {
	0, 1, 0, 0, 
	0, 1, 0, 0, 
	1, 1, 0, 0, 
    0, 0, 0, 0
};

const bool c_shape_l[] = {
	1, 0, 0, 0, 
	1, 0, 0, 0, 
	1, 1, 0, 0, 
    0, 0, 0, 0
};

const bool c_shape_s[] = {
	0, 1, 1, 0, 
	1, 1, 0, 0, 
	0, 0, 0, 0, 
    0, 0, 0, 0
};

const bool c_shape_z[] = {
	1, 1, 0, 0, 
	0, 1, 1, 0, 
	0, 0, 0, 0, 
    0, 0, 0, 0
};

const bool *c_shape_list[] = {
    c_shape_i,
    c_shape_o,
    c_shape_t,
    c_shape_j,
    c_shape_l,
    c_shape_s,
    c_shape_z
};

const uint8_t c_shape_colors[] = {
    COLOR_PAIR_CYAN_DEFAULT,
    COLOR_PAIR_YELLOW_DEFAULT,
    COLOR_PAIR_MAGENTA_DEFAULT,
    COLOR_PAIR_BLUE_DEFAULT,
    COLOR_PAIR_ORANGE_DEFAULT,
    COLOR_PAIR_GREEN_DEFAULT,
    COLOR_PAIR_RED_DEFAULT
};

#endif