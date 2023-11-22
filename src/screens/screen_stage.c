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

static const uint8_t c_win_padding = 1;
// static const uint8_t   c_score_velocity					= 30;
static const uint8_t c_speedup_velocity = 20;
// static const uint8_t   c_max_level						= 20;
static const float32_t c_shape_base_velocity			= 1; // 1 row per second
static const float32_t c_filled_rows_animation_lifetime = 0.3;

static WINDOW *win_board;
static WINDOW *win_next_shape;
static WINDOW *win_score;

static uint8_t		board[BOARD_ROWS * BOARD_COLS];
static shape_t		next_shape;
static shape_t		current_shape;
static float32_t	current_shape_elapsed_time;
static uint8_t		player_action;
static uint8_t		level;
static uint8_t		velocity;
static uint8_t		board_top_row_filled;
static sparse_set_t filled_rows_indexes;
static float32_t	filled_rows_elapsed_time;

// INIT
static void create_windows(void);
// UPDATE
static void handle_input(void);
static void move_shape(void);
static void rotate_shape(bool backward);
static void set_shape_padding(shape_t *shape);
static void drop_shape(void);
static void handle_collision(void);
static void set_shape_on_board(void);
static void scan_board_filled_rows(void);
static void process_board_filled_rows(void);
static void set_next_shape(void);
static void set_current_shape(void);
static void save_score(void);
static void update_score_labels(void);
// RENDER
static void render_win_board(void);
static void render_win_next_shape(void);
static void render_win_score(void);
static void render_shape(WINDOW *win, shape_t *shape);
static void render_board(void);

void screen_stage_init(void)
{
	g_score.record_label	   = g_score.record;
	player_action			   = PLAYER_ACTION_IDLE;
	level					   = 1;
	current_shape_elapsed_time = 0;
	board_top_row_filled	   = BOARD_ROWS - 1;
	filled_rows_indexes		   = sparse_set_new(BOARD_ROWS);
	memset(board, 0, sizeof(uint8_t) * (BOARD_ROWS * BOARD_COLS));

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

	sparse_set_dispose(&filled_rows_indexes);
}

bool screen_stage_is_completed(void)
{
	return board_top_row_filled <= 0;
}

void screen_stage_update(void)
{
	velocity = level;
	handle_input();
	move_shape();
	handle_collision();
	process_board_filled_rows();
	update_score_labels();
}

void screen_stage_render(void)
{
	render_win_next_shape();
	render_win_score();

	wclear(win_board);
	render_win_board();
	render_shape(win_board, &current_shape);
	render_board();
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

	current_shape.prev_pos.x = current_shape.pos.x;
	current_shape.prev_pos.y = current_shape.pos.y;

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
		rotate_shape(false);
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

static void rotate_shape(bool backward)
{
	bool shape_aux[SHAPE_ROWS * SHAPE_COLS];
	bool val;

	for (uint8_t y = 0; y < SHAPE_ROWS; y++)
	{
		for (uint8_t x = 0; x < SHAPE_COLS; x++)
		{
			if (backward)
			{
				val = current_shape.val[SHAPE_COLS * y + x];

				shape_aux[(SHAPE_COLS * (SHAPE_ROWS - x - 1)) + y] = val;
			}
			else
			{
				val = current_shape.val[(SHAPE_COLS * (SHAPE_ROWS - x - 1)) + y];

				shape_aux[SHAPE_COLS * y + x] = val;
			}
		}
	}

	memcpy(current_shape.val, shape_aux, SHAPE_ROWS * SHAPE_COLS);
	set_shape_padding(&current_shape);
}

static void set_shape_padding(shape_t *shape)
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
			bool fill = shape->val[SHAPE_COLS * y + x];

			if ((padding_left == -1 || x < padding_left) && fill)
			{
				padding_left = x;
			}

			// right
			fill = shape->val[SHAPE_COLS * y + (SHAPE_COLS - x - 1)];

			if ((padding_right == -1 || x < padding_right) && fill)
			{
				padding_right = x;
			}

			// top
			fill = shape->val[SHAPE_COLS * x + y];

			if ((padding_top == -1 || x < padding_top) && fill)
			{
				padding_top = x;
			}

			// bottom
			fill = shape->val[(SHAPE_COLS * (SHAPE_ROWS - x - 1)) + y];

			if ((padding_bottom == -1 || x < padding_bottom) && fill)
			{
				padding_bottom = x;
			}
		}
	}

	shape->padding_left	  = padding_left;
	shape->padding_right  = padding_right;
	shape->padding_top	  = padding_top;
	shape->padding_bottom = padding_bottom;
	shape->width		  = SHAPE_COLS - padding_left - padding_right;
	shape->height		  = SHAPE_ROWS - padding_top - padding_bottom;
}

static void drop_shape(void)
{
}

static void handle_collision(void)
{
	// side board collision
	if ((current_shape.pos.x + current_shape.padding_left) < 0)
	{
		current_shape.pos.x++;
	}
	else if ((current_shape.pos.x + current_shape.padding_left + current_shape.width) > BOARD_COLS)
	{
		current_shape.pos.x = BOARD_COLS - current_shape.width - current_shape.padding_left;
	}

	// bottom board collision
	uint8_t shape_bottom_y = current_shape.pos.y + current_shape.padding_top + current_shape.height;
	bool	y_collided	   = shape_bottom_y > BOARD_ROWS;

	// board blocks collision
	if (!y_collided && shape_bottom_y > board_top_row_filled)
	{
		for (uint8_t y = 0; y < current_shape.height && !y_collided; y++)
		{
			for (uint8_t x = 0; x < current_shape.width && !y_collided; x++)
			{
				bool	 fill = current_shape.val[SHAPE_COLS * (y + current_shape.padding_top) + (x + current_shape.padding_left)];
				uint16_t index =
					(BOARD_COLS *
						 (uint16_t)(current_shape.pos.y + current_shape.padding_top + y) +
					 (uint16_t)(current_shape.pos.x + current_shape.padding_left + x));

				y_collided = fill && board[index] > 0;
			}
		}
	}

	if (y_collided)
	{
		if (player_action == PLAYER_ACTION_ROTATE)
		{
			rotate_shape(true);
		}
		else
		{
			current_shape.pos.x = current_shape.prev_pos.x;
			current_shape.pos.y = current_shape.prev_pos.y;
		}

		if (player_action != PLAYER_ACTION_MOVE_LEFT && player_action != PLAYER_ACTION_MOVE_RIGHT && player_action != PLAYER_ACTION_ROTATE)
		{
			set_shape_on_board();
			scan_board_filled_rows();
			set_current_shape();
			set_next_shape();
		}
	}
}

static void set_shape_on_board(void)
{
	uint8_t color  = c_shape_colors[current_shape.type];
	uint8_t height = current_shape.pos.y;

	if (height < board_top_row_filled)
	{
		board_top_row_filled = height;
	}

	for (uint8_t y = 0; y < current_shape.height; y++)
	{
		for (uint8_t x = 0; x < current_shape.width; x++)
		{
			bool fill = current_shape.val[SHAPE_COLS * (y + current_shape.padding_top) + (x + current_shape.padding_left)];

			if (fill)
			{
				uint16_t index =
					BOARD_COLS *
						(uint16_t)(current_shape.pos.y + current_shape.padding_top + y) +
					(uint16_t)(current_shape.pos.x + current_shape.padding_left + x);

				board[index] = color;
			}
		}
	}
}

static void scan_board_filled_rows(void)
{
	uint8_t cells_filled_length;
	filled_rows_elapsed_time = 0;

	for (int16_t y = BOARD_ROWS - 1; y >= board_top_row_filled; y--)
	{
		cells_filled_length = 0;

		for (uint8_t x = 0; x < BOARD_COLS; x++)
		{
			cells_filled_length += board[BOARD_COLS * y + x] ? 1 : 0;
		}

		if (cells_filled_length == BOARD_COLS)
		{
			sparse_set_add(&filled_rows_indexes, (uint8_t)y);
		}
	}
}

static void process_board_filled_rows(void)
{
	uint8_t length		   = VECTOR_LENGTH(filled_rows_indexes.dense);
	uint8_t rows_to_move   = 0;
	uint8_t rows_to_remove = 0;
	size_t	size		   = 0;

	if (length == 0)
	{
		return;
	}

	filled_rows_elapsed_time += g_delta_time;

	if (filled_rows_elapsed_time < c_filled_rows_animation_lifetime)
	{
		return;
	}

	for (int16_t y = BOARD_ROWS - 1; y >= board_top_row_filled; y--)
	{
		bool row_to_remove = SPARSE_SET_CONTAINS(filled_rows_indexes, y);

		if (!row_to_remove && rows_to_remove > 0)
		{
			rows_to_move++;
		}

		if (row_to_remove || y == board_top_row_filled)
		{
			if (rows_to_move > 0)
			{
				size			= sizeof(uint8_t) * rows_to_move * BOARD_COLS;
				uint8_t *dest	= (board + ((y + rows_to_remove) * BOARD_COLS));
				uint8_t *source = (board + (y * BOARD_COLS));
				memmove(dest, source, size);

				rows_to_move = 0;
			}

			rows_to_remove++;
		}
	}

	sparse_set_clear(&filled_rows_indexes);

	if ((BOARD_ROWS - board_top_row_filled) > rows_to_remove)
	{
		rows_to_remove--;
	}

	size = sizeof(uint8_t) * rows_to_remove * BOARD_COLS;
	memset(board + (board_top_row_filled * BOARD_COLS), 0, size);
	board_top_row_filled += rows_to_remove;
}

static void set_next_shape(void)
{
	next_shape.type = rand() % SHAPES_COUNT;
	memcpy(next_shape.val, c_shape_list[next_shape.type], SHAPE_ROWS * SHAPE_COLS);
	set_shape_padding(&next_shape);

	next_shape.pos.x = (BOARD_COLS - (c_win_padding * 2)) * 0.5 - next_shape.width * 0.5 + next_shape.padding_left;
	next_shape.pos.y = c_win_next_shape_height * 0.5 - next_shape.height * 0.5 - next_shape.padding_top - c_win_padding;
}

static void set_current_shape(void)
{
	current_shape.type = next_shape.type;
	memcpy(current_shape.val, c_shape_list[current_shape.type], SHAPE_ROWS * SHAPE_COLS);
	set_shape_padding(&current_shape);

	current_shape.pos.x = floor(BOARD_COLS * 0.5 - current_shape.width * 0.5 + current_shape.padding_left);
	// current_shape.pos.x -= (uint8_t)current_shape.pos.x % 2;
	current_shape.pos.y = 0;
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
	wclear(win_next_shape);
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
				mvwprintw(win, shape->pos.y + y + c_win_padding, (shape->pos.x * 2) + (x * 2) + c_win_padding, "[]");
			}
		}
	}

	wattroff(win, COLOR_PAIR(color));
}

static void render_board(void)
{
	uint8_t color = 0;

	for (int16_t y = BOARD_ROWS - 1; y >= board_top_row_filled; y--)
	{
		for (uint8_t x = 0; x < BOARD_COLS; x++)
		{
			color = board[BOARD_COLS * y + x];

			if (!color)
			{
				continue;
			}

			bool filled_row = SPARSE_SET_CONTAINS(filled_rows_indexes, y);

			if (filled_row)
			{
				color = COLOR_PAIR_WHITE_HIGH - ((uint8_t)((filled_rows_elapsed_time) * 10) % 3);
				wattron(win_board, COLOR_PAIR(color));
				mvwaddch(win_board, y + c_win_padding, (x * 2) + c_win_padding, CH_SHAPE_FILL);
				mvwaddch(win_board, y + c_win_padding, (x * 2) + 1 + c_win_padding, CH_SHAPE_FILL);
				wattroff(win_board, COLOR_PAIR(color));
			}
			else
			{
				wattron(win_board, COLOR_PAIR(color));
				mvwaddch(win_board, y + c_win_padding, (x * 2) + c_win_padding, '[');
				mvwaddch(win_board, y + c_win_padding, (x * 2) + 1 + c_win_padding, ']');
				wattroff(win_board, COLOR_PAIR(color));
			}
		}
	}
}