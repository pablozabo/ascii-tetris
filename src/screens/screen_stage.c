#include "screen_stage.h"
#include "../common.h"
#include "../data_structures/data_structures.h"
#include "../shapes.h"

extern int		 g_key;
extern score_t	 g_score;
extern float32_t g_delta_time;

#define BOARD_ROWS 20
#define BOARD_COLS 10

typedef enum player_action_t
{
	PLAYER_ACTION_IDLE		 = 0,
	PLAYER_ACTION_MOVE_LEFT	 = 1,
	PLAYER_ACTION_MOVE_RIGHT = 2,
	PLAYER_ACTION_ROTATE	 = 3,
	PLAYER_ACTION_SPEEDUP	 = 4,
	PLAYER_ACTION_HARD_DROP	 = 5,
} player_action_t;

static const uint8_t c_win_board_width		 = 22;
static const uint8_t c_win_board_height		 = 22;
static const uint8_t c_win_next_shape_width	 = 20;
static const uint8_t c_win_next_shape_height = 11;
static const uint8_t c_win_score_width		 = 20;
static const uint8_t c_win_score_height		 = 11;

static const uint8_t   c_win_padding		 = 1;
static const uint8_t   c_score_velocity		 = 30;
static const uint8_t   c_speedup_velocity	 = 20;
static const uint8_t   c_max_level			 = 20;
static const float32_t c_shape_base_velocity = 1; // 1 row per second

static WINDOW *win_board;
static WINDOW *win_next_shape;
static WINDOW *win_score;

static uint8_t	 board[BOARD_ROWS * BOARD_COLS];
static shape_t	 next_shape;
static shape_t	 current_shape;
static float32_t current_shape_elapsed_time;
static uint8_t	 player_action;
static uint8_t	 level;
static uint8_t	 velocity;
static uint8_t	 board_height; // number of rows with one or more filled cells

// init
static void create_windows(void);
// update
static void handle_input(void);
static void move_shape(void);
static void rotate_shape(void);
static void set_shape_padding(void);
static void drop_shape(void);
static void handle_collision(void);
static void process_board_rows(void);
static void set_next_shape(void);
static void set_current_shape(void);
static void save_score(void);
static void update_score_labels(void);
// render
static void render_win_board(void);
static void render_win_next_shape(void);
static void render_win_score(void);
static void render_shape(WINDOW *win, shape_t *shape);

void screen_stage_init(void)
{
	g_score.record_label	   = g_score.record;
	player_action			   = PLAYER_ACTION_IDLE;
	level					   = 1;
	current_shape_elapsed_time = 0;
	board_height			   = 0;

	srand(time(NULL));
	create_windows();
	set_next_shape();
	set_current_shape();
	set_next_shape();

	render_win_board();
	render_win_next_shape();
	render_win_score();
}

void screen_stage_dispose(void)
{
	save_score();
	wclear(win_board);
	wrefresh(win_board);
	delwin(win_board);

	wclear(win_next_shape);
	wrefresh(win_next_shape);
	delwin(win_next_shape);

	wclear(win_score);
	wrefresh(win_score);
	delwin(win_score);
}

bool screen_stage_is_completed(void)
{
	return false;
}

void screen_stage_update(void)
{
	velocity = level;
	handle_input();
	move_shape();
	handle_collision();
	update_score_labels();
}

void screen_stage_render(void)
{
	render_win_next_shape();
	render_win_score();

	wclear(win_board);
	render_win_board();
	render_shape(win_board, &current_shape);
	wrefresh(win_board);
}

// UPDATE
static void create_windows(void)
{
	uint8_t offset_y, offset_x;

	set_offset_yx(c_win_board_height, c_win_board_width + c_win_next_shape_width, &offset_y, &offset_x);
	win_board = newwin(c_win_board_height, c_win_board_width, offset_y, offset_x);
	scrollok(win_board, TRUE);

	win_next_shape = newwin(c_win_next_shape_height, c_win_next_shape_width, offset_y, offset_x + c_win_board_width);
	scrollok(win_next_shape, TRUE);

	win_score = newwin(c_win_score_height, c_win_score_width, offset_y + c_win_next_shape_height, offset_x + c_win_board_width);
	scrollok(win_score, TRUE);
}

static void handle_input(void)
{
	player_action = PLAYER_ACTION_IDLE;

	if (g_key > 0)
	{
		if (g_key == KEY_LEFT)
		{
			player_action = PLAYER_ACTION_MOVE_LEFT;
		}
		else if (g_key == KEY_RIGHT)
		{
			player_action = PLAYER_ACTION_MOVE_RIGHT;
		}
		else if (g_key == KEY_UP)
		{
			player_action = PLAYER_ACTION_ROTATE;
		}
		else if (g_key == KEY_DOWN && level < c_speedup_velocity)
		{
			player_action = PLAYER_ACTION_SPEEDUP;
		}
		else if (g_key == CH_SPACE)
		{
			player_action = PLAYER_ACTION_HARD_DROP;
		}
	}
}

static void move_shape(void)
{
	current_shape_elapsed_time += g_delta_time;

	if (player_action == PLAYER_ACTION_MOVE_LEFT)
	{
		current_shape.pos.x -= 1;
	}
	else if (player_action == PLAYER_ACTION_MOVE_RIGHT)
	{
		current_shape.pos.x += 1;
	}
	else if (player_action == PLAYER_ACTION_ROTATE)
	{
		rotate_shape();
	}
	else if (player_action == PLAYER_ACTION_SPEEDUP)
	{
		velocity = c_speedup_velocity;
	}

	if (player_action == PLAYER_ACTION_HARD_DROP)
	{
		drop_shape();
	}
	else if (current_shape_elapsed_time >= (c_shape_base_velocity - (velocity * 0.05)))
	{
		current_shape_elapsed_time = 0;
		current_shape.pos.y += 1;
	}
}

static void rotate_shape(void)
{
	bool shape_aux[SHAPE_ROWS * SHAPE_COLS];

	for (uint8_t y = 0; y < SHAPE_ROWS; y++)
	{
		for (uint8_t x = 0; x < SHAPE_COLS; x++)
		{
			bool fill = current_shape.val[(SHAPE_COLS * (SHAPE_ROWS - x - 1)) + y];

			shape_aux[SHAPE_COLS * y + x] = fill;
		}
	}

	memcpy(current_shape.val, shape_aux, SHAPE_ROWS * SHAPE_COLS);
	set_shape_padding();
}

static void set_shape_padding(void)
{
	int8_t padding_left	  = -1;
	int8_t padding_right  = -1;
	int8_t padding_top	  = -1;
	int8_t padding_bottom = -1;

	for (uint8_t y = 0; y < SHAPE_ROWS; y++)
	{
		for (uint8_t x = 0; x < SHAPE_COLS; x++)
		{
			// left
			bool fill = current_shape.val[SHAPE_COLS * y + x];

			if ((padding_left == -1 || x < padding_left) && fill)
			{
				padding_left = x;
			}

			// right
			fill = current_shape.val[SHAPE_COLS * y + (SHAPE_COLS - x - 1)];

			if ((padding_right == -1 || x < padding_right) && fill)
			{
				padding_right = x;
			}

			// top
			fill = current_shape.val[SHAPE_COLS * x + y];

			if ((padding_top == -1 || x < padding_top) && fill)
			{
				padding_top = x;
			}

			// bottom
			fill = current_shape.val[(SHAPE_COLS * (SHAPE_ROWS - x - 1)) + y];

			if ((padding_bottom == -1 || x < padding_bottom) && fill)
			{
				padding_bottom = x;
			}
		}
	}

	current_shape.padding_left	 = padding_left;
	current_shape.padding_right	 = padding_right;
	current_shape.padding_top	 = padding_top;
	current_shape.padding_bottom = padding_bottom;
	current_shape.width			 = SHAPE_COLS - padding_left - padding_right;
}

static void drop_shape(void)
{
}

static void handle_collision(void)
{
	uint8_t windows_width_no_padding = c_win_board_width - (c_win_padding * 2);

	if (current_shape.pos.x < 0)
	{
		current_shape.pos.x++;
	}
	else if ((current_shape.pos.x + (current_shape.width * 2)) >= windows_width_no_padding)
	{
		current_shape.pos.x = windows_width_no_padding - (current_shape.width * 2);
	}
}

static void process_board_rows(void)
{
}

static void set_next_shape(void)
{
	next_shape.type	 = rand() % SHAPES_COUNT;
	next_shape.pos.x = c_win_next_shape_width * 0.5 - SHAPE_COLS * 0.5 - c_win_padding;
	next_shape.pos.y = c_win_next_shape_height * 0.5 - SHAPE_ROWS * 0.5;
	memcpy(next_shape.val, c_shape_list[next_shape.type], SHAPE_ROWS * SHAPE_COLS);
}

static void set_current_shape(void)
{
	current_shape.type	= next_shape.type;
	current_shape.pos.x = c_win_board_width * 0.5 - SHAPE_COLS * 0.5;
	current_shape.pos.y = 0;
	memcpy(current_shape.val, c_shape_list[current_shape.type], SHAPE_ROWS * SHAPE_COLS);
	set_shape_padding();
}

static void save_score(void)
{
	if (g_score.current <= g_score.record)
	{
		return;
	}

	g_score.record = g_score.current;

	FILE *f = fopen(FILE_SCORE, "w");
	ASSERT(f);

	fprintf(f, "%d", g_score.record);
	fclose(f);
}

static void update_score_labels(void)
{
	// if (g_score.current_stage_label < g_score.current_stage)
	// {
	// 	g_score.current_stage_label += c_score_velocity * g_delta_time;

	// 	if (g_score.current_stage_label > g_score.current_stage)
	// 	{
	// 		g_score.current_stage_label = g_score.current_stage;
	// 	}
	// }

	// if (g_score.record_label < g_score.record)
	// {
	// 	g_score.record_label += c_score_velocity * g_delta_time;

	// 	if (g_score.record_label > g_score.record)
	// 	{
	// 		g_score.record_label = g_score.record;
	// 	}
	// }
}

// RENDER
static void render_win_board(void)
{
	wattron(win_board, COLOR_PAIR(COLOR_PAIR_MAGENTA_MEDIUM));
	box(win_board, 0, 0);
	wattroff(win_board, COLOR_PAIR(COLOR_PAIR_MAGENTA_MEDIUM));
}

static void render_win_next_shape()
{
	char		level[3]	= { '\0' };
	const char *title		= "NEXT";
	const char *lines_label = "Level:";

	uint8_t padding_x = c_win_padding * 2,
			padding_y = c_win_next_shape_height - 2;
	uint8_t l		  = 0;

	sprintf(level, "%d", l);

	wattron(win_next_shape, COLOR_PAIR(COLOR_PAIR_MAGENTA_MEDIUM));
	box(win_next_shape, 0, 0);
	wattroff(win_next_shape, COLOR_PAIR(COLOR_PAIR_MAGENTA_MEDIUM));

	// title
	wattron(win_next_shape, COLOR_PAIR(COLOR_PAIR_MAGENTA_HIGH));
	mvwprintw(win_next_shape, 0, (c_win_next_shape_width * 0.5) - floor(strlen(title) * 0.5), "%s", title);
	wattroff(win_next_shape, COLOR_PAIR(COLOR_PAIR_MAGENTA_HIGH));
	// level
	wattron(win_next_shape, COLOR_PAIR(COLOR_PAIR_GREEN_DEFAULT));
	mvwprintw(win_next_shape, padding_y, padding_x, "%s", lines_label);
	mvwprintw(win_next_shape, padding_y, strlen(lines_label) + padding_x + 1, "%s", level);
	wattroff(win_next_shape, COLOR_PAIR(COLOR_PAIR_GREEN_DEFAULT));
	// shape
	render_shape(win_next_shape, &next_shape);

	wrefresh(win_next_shape);
}

static void render_win_score()
{
	char		max_score[10]		= { '\0' };
	char		current_score[10]	= { '\0' };
	const char *title				= "SCORE";
	const char *current_score_label = "Current:";
	const char *max_score_label		= "Top:";

	uint8_t padding_x = c_win_padding * 2,
			padding_y = c_win_padding * 2;

	sprintf(current_score, "%d", (uint16_t)(g_score.current_label));
	sprintf(max_score, "%d", (uint16_t)(g_score.record_label));

	wattron(win_score, COLOR_PAIR(COLOR_PAIR_MAGENTA_MEDIUM));
	box(win_score, 0, 0);
	wattroff(win_score, COLOR_PAIR(COLOR_PAIR_MAGENTA_MEDIUM));

	// title
	wattron(win_score, COLOR_PAIR(COLOR_PAIR_MAGENTA_HIGH));
	mvwprintw(win_score, 0, (c_win_score_width * 0.5) - floor(strlen(title) * 0.5), "%s", title);
	wattroff(win_score, COLOR_PAIR(COLOR_PAIR_MAGENTA_HIGH));
	// current score
	mvwprintw(win_score, padding_y, padding_x, "%s", current_score_label);
	mvwprintw(win_score, padding_y, strlen(current_score_label) + padding_x + 1, "%s", current_score);
	// max score
	wattron(win_score, COLOR_PAIR(COLOR_PAIR_GREEN_DEFAULT));
	mvwprintw(win_score, padding_y + 1, padding_x, "%s", max_score_label);
	mvwprintw(win_score, padding_y + 1, strlen(current_score_label) + padding_x + 1, "%s", max_score);
	wattroff(win_score, COLOR_PAIR(COLOR_PAIR_GREEN_DEFAULT));

	wrefresh(win_score);
}

static void render_shape(WINDOW *win, shape_t *shape)
{
	uint8_t color = c_shape_colors[shape->type];

	wattron(win, COLOR_PAIR(color));

	for (uint8_t y = 0; y < SHAPE_ROWS; y++)
	{
		for (uint8_t x = 0; x < SHAPE_COLS; x++)
		{
			bool fill = shape->val[SHAPE_COLS * y + x];

			if (fill)
			{
				mvwprintw(win, shape->pos.y + y + c_win_padding, shape->pos.x + (x * 2) + c_win_padding - (shape->padding_left * 2), "[]");
			}
		}
	}

	wattroff(win, COLOR_PAIR(color));
}