#ifndef SHAPES_H
#define SHAPES_H

#include "defs.h"
#include "vec2.h"

#define SHAPE_ROWS 4
#define SHAPE_COLS 4
#define SHAPES_COUNT 7

const bool c_shape_i[] = {
	1, 0, 0, 0,
	1, 0, 0, 0,
	1, 0, 0, 0,
	1, 0, 0, 0
};

const bool c_shape_o[] = {
	0, 0, 0, 0,
	1, 1, 0, 0,
	1, 1, 0, 0,
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
	COLOR_PAIR_WHITE_DEFAULT,
	COLOR_PAIR_BLUE_DEFAULT,
	COLOR_PAIR_ORANGE_DEFAULT,
	COLOR_PAIR_GREEN_DEFAULT,
	COLOR_PAIR_RED_DEFAULT
};

typedef enum shape_type_t
{
	SHAPE_TYPE_I = 0,
	SHAPE_TYPE_O = 1,
	SHAPE_TYPE_T = 2,
	SHAPE_TYPE_J = 3,
	SHAPE_TYPE_L = 4,
	SHAPE_TYPE_S = 5,
	SHAPE_TYPE_Z = 6
} shape_type_t;

typedef struct shape_t
{
	vec2_t		 pos;
	shape_type_t type;
	bool		 val[SHAPE_ROWS * SHAPE_COLS];
} shape_t;

#endif