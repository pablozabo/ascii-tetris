#ifndef VEC2_H
#define VEC2_H

#include "defs.h"

#define MATH_PI 3.14159
#define RADIANS_TO_DEGREES(radians) (radians * (180 / MATH_PI))
#define DEGREES_TO_RADIANS(degrees) (degrees * (MATH_PI / 180))

typedef struct vec2_t
{
	float32_t x;
	float32_t y;
} vec2_t;

void   vec2_add_to(vec2_t *vec, vec2_t val);
vec2_t vec2_add(vec2_t val1, vec2_t val2);
void   vec2_sub_to(vec2_t *vec, vec2_t val);
vec2_t vec2_sub(vec2_t val1, vec2_t val2);
void   vec2_mul_to(vec2_t *vec, vec2_t val);
vec2_t vec2_mul(vec2_t val1, vec2_t val2);
void   vec2_mul_scalar_to(vec2_t *vec, float32_t val);
vec2_t vec2_mul_sacalar(vec2_t val1, float32_t val2);
void   vec2_div_to(vec2_t *vec, vec2_t val);
vec2_t vec2_div(vec2_t val1, vec2_t val2);
void   vec2_div_scalar_to(vec2_t *vec, float32_t val);
vec2_t vec2_div_sacalar(vec2_t val1, float32_t val2);
void   vec2_normalize_to(vec2_t *vec);
vec2_t vec2_normalize(vec2_t val);
void   vec2_reflect(vec2_t *normal_vec, vec2_t normal_val);
vec2_t vec2_direction(vec2_t normal_vec);

float32_t vec2_length(vec2_t vec);
float32_t vec2_dot(vec2_t vec1, vec2_t vec2);
float32_t vec2_cross(vec2_t vec1, vec2_t vec2);

#endif