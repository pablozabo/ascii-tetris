#ifndef DEFS_H
#define DEFS_H

#include <curses.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// TYPES
typedef signed char	   int8_t;
typedef unsigned char  uint8_t;
typedef signed short   int16_t;
typedef unsigned short uint16_t;
typedef int			   int32_t;
typedef unsigned	   uint32_t;
typedef float		   float32_t;
typedef void (*screen_action_t)(void);
typedef bool (*screen_is_completed_t)(void);

// DEFS
#define CURRENT_TIME (1000.0 * clock() / CLOCKS_PER_SEC)

#define CH_SHAPE_SOLID_FILL ACS_BLOCK
#define CH_SHAPE_FILL ACS_CKBOARD
#define CH_SHAPE_SPINE ACS_BTEE
#define CH_EOL '\n'
#define CH_EOS '\0'
#define CH_ENTER 10
#define CH_CIRCLE 'O'
#define CH_DOT '.'
#define CH_ESC 27
#define CH_SPACE 32
#define CH_LEFT KEY_LEFT
#define CH_RIGHT KEY_RIGHT
#define CH_UP KEY_UP
#define CH_DOWN KEY_DOWN
#define CH_PAUSE_L 'p'
#define CH_PAUSE_U 'P'
#define CH_SHAPE_SHADOW_L 's'
#define CH_SHAPE_SHADOW_U 'S'

#define FILE_SCORE "score.txt"

typedef enum color_pair_t
{
	COLOR_PAIR_RED_DEFAULT	   = 1,
	COLOR_PAIR_BLUE_DEFAULT	   = 2,
	COLOR_PAIR_GREEN_DEFAULT   = 3,
	COLOR_PAIR_YELLOW_DEFAULT  = 4,
	COLOR_PAIR_ORANGE_DEFAULT  = 5,
	COLOR_PAIR_CYAN_DEFAULT	   = 6,
	COLOR_PAIR_MAGENTA_DEFAULT = 7,
	COLOR_PAIR_WHITE_DEFAULT   = 8,

	COLOR_PAIR_RED_LOW	  = 10,
	COLOR_PAIR_RED_MEDIUM = 11,
	COLOR_PAIR_RED_HIGH	  = 12,

	COLOR_PAIR_BLUE_LOW	   = 20,
	COLOR_PAIR_BLUE_MEDIUM = 21,
	COLOR_PAIR_BLUE_HIGH   = 22,

	COLOR_PAIR_GREEN_LOW	= 30,
	COLOR_PAIR_GREEN_MEDIUM = 31,
	COLOR_PAIR_GREEN_HIGH	= 32,

	COLOR_PAIR_YELLOW_LOW	 = 40,
	COLOR_PAIR_YELLOW_MEDIUM = 41,
	COLOR_PAIR_YELLOW_HIGH	 = 42,

	COLOR_PAIR_ORANGE_LOW	 = 50,
	COLOR_PAIR_ORANGE_MEDIUM = 51,
	COLOR_PAIR_ORANGE_HIGH	 = 52,

	COLOR_PAIR_CYAN_LOW	   = 60,
	COLOR_PAIR_CYAN_MEDIUM = 61,
	COLOR_PAIR_CYAN_HIGH   = 62,

	COLOR_PAIR_MAGENTA_LOW	  = 70,
	COLOR_PAIR_MAGENTA_MEDIUM = 71,
	COLOR_PAIR_MAGENTA_HIGH	  = 72,

	COLOR_PAIR_WHITE_LOW	= 80,
	COLOR_PAIR_WHITE_MEDIUM = 81,
	COLOR_PAIR_WHITE_HIGH	= 82,

	COLOR_PAIR_BLUE_BK = 90,
	COLOR_PAIR_RED_BK  = 91
} color_pair_t;

typedef enum custom_color_t
{
	CUSTOM_COLOR_RED_DEFAULT = 16,
	CUSTOM_COLOR_RED_LOW	 = 17,
	CUSTOM_COLOR_RED_MEDIUM	 = 18,
	CUSTOM_COLOR_RED_HIGH	 = 19,

	CUSTOM_COLOR_BLUE_DEFAULT = 20,
	CUSTOM_COLOR_BLUE_LOW	  = 21,
	CUSTOM_COLOR_BLUE_MEDIUM  = 22,
	CUSTOM_COLOR_BLUE_HIGH	  = 23,

	CUSTOM_COLOR_GREEN_DEFAULT = 24,
	CUSTOM_COLOR_GREEN_LOW	   = 25,
	CUSTOM_COLOR_GREEN_MEDIUM  = 26,
	CUSTOM_COLOR_GREEN_HIGH	   = 27,

	CUSTOM_COLOR_YELLOW_DEFAULT = 28,
	CUSTOM_COLOR_YELLOW_LOW		= 29,
	CUSTOM_COLOR_YELLOW_MEDIUM	= 30,
	CUSTOM_COLOR_YELLOW_HIGH	= 31,

	CUSTOM_COLOR_ORANGE_DEFAULT = 32,
	CUSTOM_COLOR_ORANGE_LOW		= 33,
	CUSTOM_COLOR_ORANGE_MEDIUM	= 34,
	CUSTOM_COLOR_ORANGE_HIGH	= 35,

	CUSTOM_COLOR_CYAN_DEFAULT = 36,
	CUSTOM_COLOR_CYAN_LOW	  = 37,
	CUSTOM_COLOR_CYAN_MEDIUM  = 38,
	CUSTOM_COLOR_CYAN_HIGH	  = 39,

	CUSTOM_COLOR_MAGENTA_DEFAULT = 40,
	CUSTOM_COLOR_MAGENTA_LOW	 = 41,
	CUSTOM_COLOR_MAGENTA_MEDIUM	 = 42,
	CUSTOM_COLOR_MAGENTA_HIGH	 = 43,

	CUSTOM_COLOR_WHITE_DEFAULT = 44,
	CUSTOM_COLOR_WHITE_LOW	   = 45,
	CUSTOM_COLOR_WHITE_MEDIUM  = 46,
	CUSTOM_COLOR_WHITE_HIGH	   = 47
} custom_color_t;

typedef struct score_t
{
	uint16_t current;
	uint16_t record;
	// next members are used to show animated score on screen
	uint16_t current_label;
	uint16_t record_label;
} score_t;

typedef struct vec2_t
{
	float32_t x;
	float32_t y;
} vec2_t;

#endif