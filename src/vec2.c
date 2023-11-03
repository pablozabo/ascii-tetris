#include "vec2.h"

vec2_t directions[] = {
	{ .x = 0, .y = -1 }, // up
	{ .x = 0, .y = 1 },	 // down
	{ .x = 1, .y = 0 },	 // right
	{ .x = -1, .y = 0 }	 // left
};

void vec2_add_to(vec2_t *target, vec2_t val)
{
	target->x += val.x;
	target->y += val.y;
}

vec2_t vec2_add(vec2_t val1, vec2_t val2)
{
	vec2_t result = { .x = val1.x + val2.x, .y = val1.y + val2.y };
	return result;
}

void vec2_sub_to(vec2_t *target, vec2_t val)
{
	target->x -= val.x;
	target->y -= val.y;
}

vec2_t vec2_sub(vec2_t val1, vec2_t val2)
{
	vec2_t result = { .x = val2.x - val1.x, .y = val2.y - val1.y };
	return result;
}

void vec2_mul_to(vec2_t *target, vec2_t val)
{
	target->x *= val.x;
	target->y *= val.y;
}

vec2_t vec2_mul(vec2_t val1, vec2_t val2)
{
	vec2_t result = { .x = val1.x * val2.x, .y = val1.y * val2.y };
	return result;
}

void vec2_mul_scalar_to(vec2_t *target, float32_t val)
{
	target->x *= val;
	target->y *= val;
}

vec2_t vec2_mul_sacalar(vec2_t val1, float32_t val2)
{
	vec2_t result = { .x = val1.x * val2, .y = val1.y * val2 };
	return result;
}

void vec2_div_to(vec2_t *target, vec2_t val)
{
	target->x /= val.x;
	target->y /= val.y;
}

vec2_t vec2_div(vec2_t val1, vec2_t val2)
{
	vec2_t result = { .x = val2.x / val1.x, .y = val2.y / val1.y };
	return result;
}

void vec2_div_scalar_to(vec2_t *target, float32_t val)
{
	target->x /= val;
	target->y /= val;
}

vec2_t vec2_div_sacalar(vec2_t val1, float32_t val2)
{
	vec2_t result = { .x = val1.x / val2, .y = val1.y / val2 };
	return result;
}

void vec2_normalize_to(vec2_t *target)
{
	float32_t length = vec2_length(*target);

	if (length > 0)
	{
		vec2_div_scalar_to(target, length);
	}
}

vec2_t vec2_normalize(vec2_t val)
{
	float32_t length = vec2_length(val);
	vec2_t	  result;

	if (length > 0)
	{
		result.x = val.x;
		result.y = val.y;

		vec2_div_scalar_to(&result, length);
	}

	return result;
}

void vec2_reflect(vec2_t *normal_target, vec2_t normal_val)
{
	// R = I - 2 * (I dot N)*N
	float32_t dot = vec2_dot(*normal_target, normal_val);
	vec2_mul_scalar_to(&normal_val, dot * 2);
	vec2_sub_to(normal_target, normal_val);
}

vec2_t vec2_direction(vec2_t normal_vec)
{
	float32_t max = 0;
	vec2_t	  result;

	for (uint8_t i = 0; i < 4; i++)
	{
		float32_t dot = vec2_dot(normal_vec, directions[i]);

		if (dot > max)
		{
			max	   = dot;
			result = directions[i];
		}
	}

	return result;
}

float32_t vec2_length(vec2_t vec)
{
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float32_t vec2_dot(vec2_t vec1, vec2_t vec2)
{
	return vec1.x * vec2.x + vec1.y * vec2.y;
}

float32_t vec2_cross(vec2_t vec1, vec2_t vec2)
{
	return sqrt(vec1.x * vec2.y - vec1.y * vec2.x);
}
