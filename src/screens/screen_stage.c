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
	PLAYER_ACTION_HARD_DROP	 = 5
} player_action_t;

static const uint8_t c_win_board_width		 = 22;
static const uint8_t c_win_board_height		 = 22;
static const uint8_t c_win_next_shape_width	 = 20;
static const uint8_t c_win_next_shape_height = 11;
static const uint8_t c_win_score_width		 = 20;
static const uint8_t c_win_score_height		 = 11;
static const uint8_t c_win_paused_width		 = 20;
static const uint8_t c_win_paused_height	 = 9;

static const uint8_t   c_win_padding					= 1;
static const uint8_t   c_score_velocity					= 30;
static const uint8_t   c_speedup_velocity				= 20;
static const uint8_t   c_max_level						= 10;
static const float32_t c_shape_base_velocity			= 1; // 1 row per second
static const float32_t c_filled_rows_animation_lifetime = 0.3;
static const float32_t c_prev_shape_animation_lifetime	= 0.3;
static const float32_t c_game_over_filled_rows_velocity = 0.05;

static WINDOW *win_board;
static WINDOW *win_next_shape;
static WINDOW *win_score;
static WINDOW *win_paused;

static uint8_t board[BOARD_ROWS * BOARD_COLS];
static shape_t next_shape;
static shape_t current_shape;
static shape_t prev_shape;

static float32_t	current_shape_elapsed_time;
static uint8_t		player_action;
static uint8_t		level;
static uint8_t		velocity;
static uint8_t		board_top_row_filled;
static sparse_set_t filled_rows_indexes;
static float32_t	filled_rows_elapsed_time;
static float32_t	prev_shape_elapsed_time;
static uint8_t		game_over_filled_rows;
static float32_t	game_over_filled_rows_elapsed_time;

static bool shape_shadow_enabled;
static bool paused;
static bool win_paused_active;

// INIT
static void create_windows(void);
// UPDATE
static void handle_input(void);
static void update_current_shape(void);
static void rotate_shape(bool backward);
static void set_shape_padding(shape_t *shape);
static void drop_shape(void);
static void handle_collision(void);
static void set_shape_on_board(void);
static void scan_board_filled_rows(void);
static void process_board_filled_rows(void);
static void process_game_over_filled_rows(void);
static void process_prev_shape_animation(void);
static void set_prev_shape(void);
static void set_next_shape(void);
static void set_current_shape(void);
static void save_score(void);
static void update_score_labels(void);

static uint8_t get_shape_dest_pos_y(void);
// RENDER
static void render_win_board(void);
static void render_win_next_shape(void);
static void render_win_score(void);
static void render_win_paused(void);
static void render_shape(WINDOW *win, shape_t *shape, bool shadow);
static void render_board(void);

void screen_stage_init(void)
{
	g_score.current		  = 0;
	g_score.current_label = 0;
	g_score.record_label  = g_score.record;

	paused							   = false;
	win_paused_active				   = false;
	player_action					   = PLAYER_ACTION_IDLE;
	level							   = 1;
	current_shape_elapsed_time		   = 0;
	board_top_row_filled			   = BOARD_ROWS - 1;
	filled_rows_indexes				   = sparse_set_new(BOARD_ROWS);
	game_over_filled_rows			   = 0;
	game_over_filled_rows_elapsed_time = 0;

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

	wclear(win_paused);
	wrefresh(win_paused);
	delwin(win_paused);

	sparse_set_dispose(&filled_rows_indexes);
}

bool screen_stage_is_completed(void)
{
	return board_top_row_filled == 0 && game_over_filled_rows >= BOARD_ROWS;
}

void screen_stage_update(void)
{
	if (board_top_row_filled > 0)
	{
		velocity = level;
		handle_input();

		if (!paused)
		{
			update_current_shape();
			handle_collision();
			process_board_filled_rows();
			process_prev_shape_animation();
			update_score_labels();
		}
	}
	else
	{
		process_game_over_filled_rows();
	}
}

void screen_stage_render(void)
{
	if (paused && !win_paused_active)
	{
		render_win_paused();
	}
	else if (!paused)
	{
		render_win_next_shape();
		render_win_score();

		wclear(win_board);
		render_win_board();
		render_shape(win_board, &current_shape, shape_shadow_enabled);
		render_board();
		wrefresh(win_board);
	}
}

void screen_stage_window_resized(void)
{
	render_win_next_shape();
	render_win_score();

	wclear(win_board);
	render_win_board();
	render_shape(win_board, &current_shape, shape_shadow_enabled);
	render_board();
	wrefresh(win_board);

	if (paused)
	{
		render_win_paused();
	}
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

	set_offset_yx(c_win_paused_height, c_win_paused_width, &offset_y, &offset_x);
	win_paused = newwin(c_win_paused_height, c_win_paused_width, offset_y, offset_x);
	scrollok(win_score, TRUE);
}

static void handle_input(void)
{
	player_action = PLAYER_ACTION_IDLE;

	if (g_key > 0)
	{
		if (g_key == CH_LEFT)
		{
			player_action = PLAYER_ACTION_MOVE_LEFT;
		}
		else if (g_key == CH_RIGHT)
		{
			player_action = PLAYER_ACTION_MOVE_RIGHT;
		}
		else if (g_key == CH_UP)
		{
			player_action = PLAYER_ACTION_ROTATE;
		}
		else if (g_key == CH_DOWN && level < c_speedup_velocity)
		{
			player_action = PLAYER_ACTION_SPEEDUP;
		}
		else if (g_key == CH_SPACE)
		{
			player_action = PLAYER_ACTION_HARD_DROP;
		}
		else if (g_key == CH_PAUSE_L || g_key == CH_PAUSE_U)
		{
			paused			  = !paused;
			win_paused_active = false;
		}
		else if (g_key == CH_SHAPE_SHADOW_L || g_key == CH_SHAPE_SHADOW_U)
		{
			shape_shadow_enabled = !shape_shadow_enabled;
		}
	}
}

static void update_current_shape(void)
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
	else if (current_shape_elapsed_time >= (c_shape_base_velocity - (velocity * 0.1)))
	{
		current_shape_elapsed_time = 0;
		current_shape.pos.y += 1;
	}

	if (shape_shadow_enabled)
	{
		current_shape.shadow_pos_y = get_shape_dest_pos_y();
	}
}

static void rotate_shape(bool backward)
{
	uint8_t shape_size = c_shape_size[current_shape.type];
	bool	shape_aux[SHAPE_MAX_SIZE * SHAPE_MAX_SIZE];
	bool	val;

	for (uint8_t y = 0; y < shape_size; y++)
	{
		for (uint8_t x = 0; x < shape_size; x++)
		{
			if (backward)
			{
				val = current_shape.val[shape_size * y + x];

				shape_aux[(shape_size * (shape_size - x - 1)) + y] = val;
			}
			else
			{
				val = current_shape.val[(shape_size * (shape_size - x - 1)) + y];

				shape_aux[shape_size * y + x] = val;
			}
		}
	}

	memcpy(current_shape.val, shape_aux, shape_size * shape_size);
	set_shape_padding(&current_shape);
}

static void set_shape_padding(shape_t *shape)
{
	int8_t	padding_left   = -1;
	int8_t	padding_right  = -1;
	int8_t	padding_top	   = -1;
	int8_t	padding_bottom = -1;
	uint8_t shape_size	   = c_shape_size[shape->type];

	for (uint8_t y = 0; y < shape_size; y++)
	{
		for (uint8_t x = 0; x < shape_size; x++)
		{
			// left
			bool filled = shape->val[shape_size * y + x];

			if ((padding_left == -1 || x < padding_left) && filled)
			{
				padding_left = x;
			}

			// right
			filled = shape->val[shape_size * y + (shape_size - x - 1)];

			if ((padding_right == -1 || x < padding_right) && filled)
			{
				padding_right = x;
			}

			// top
			filled = shape->val[shape_size * x + y];

			if ((padding_top == -1 || x < padding_top) && filled)
			{
				padding_top = x;
			}

			// bottom
			filled = shape->val[(shape_size * (shape_size - x - 1)) + y];

			if ((padding_bottom == -1 || x < padding_bottom) && filled)
			{
				padding_bottom = x;
			}
		}
	}

	shape->padding_left	  = padding_left;
	shape->padding_right  = padding_right;
	shape->padding_top	  = padding_top;
	shape->padding_bottom = padding_bottom;
	shape->width		  = shape_size - padding_left - padding_right;
	shape->height		  = shape_size - padding_top - padding_bottom;
}

static void drop_shape(void)
{
	current_shape.pos.y = get_shape_dest_pos_y();
	set_shape_on_board();
	scan_board_filled_rows();
	set_prev_shape();
	set_current_shape();
	set_next_shape();
}

static uint8_t get_shape_dest_pos_y(void)
{
	uint8_t board_floor	 = BOARD_ROWS;
	uint8_t shape_left_x = current_shape.pos.x + current_shape.padding_left;
	uint8_t shape_size	 = c_shape_size[current_shape.type];
	bool	collided	 = false;
	uint8_t result;

	for (int16_t y = current_shape.pos.y; y <= board_floor && !collided; y++)
	{
		for (uint8_t y_shape = 0; y_shape < current_shape.height && !collided; y_shape++)
		{
			for (uint8_t x_shape = 0; x_shape < current_shape.width && !collided; x_shape++)
			{
				bool shape_filled = current_shape.val[shape_size * (y_shape + current_shape.padding_top) + (x_shape + current_shape.padding_left)];
				bool board_filled = board[BOARD_COLS * (y + y_shape + current_shape.padding_top) + (shape_left_x + x_shape)] > 0;
				collided		  = (shape_filled && board_filled) || (y + current_shape.padding_top + current_shape.height) > board_floor;

				if (collided)
				{
					result = y - 1;
				}
			}
		}
	}

	return result;
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
	uint8_t shape_size	   = c_shape_size[current_shape.type];

	// board blocks collision
	if (!y_collided && shape_bottom_y > board_top_row_filled)
	{
		for (uint8_t y = 0; y < current_shape.height && !y_collided; y++)
		{
			for (uint8_t x = 0; x < current_shape.width && !y_collided; x++)
			{
				bool	 filled = current_shape.val[shape_size * (y + current_shape.padding_top) + (x + current_shape.padding_left)];
				uint16_t index =
					(BOARD_COLS *
						 (uint16_t)(current_shape.pos.y + current_shape.padding_top + y) +
					 (uint16_t)(current_shape.pos.x + current_shape.padding_left + x));

				y_collided = filled && board[index] > 0;
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
			set_prev_shape();
			set_current_shape();
			set_next_shape();
		}
	}
}

static void set_shape_on_board(void)
{
	uint8_t color	   = c_shape_colors[current_shape.type];
	uint8_t shape_size = c_shape_size[current_shape.type];
	uint8_t height	   = current_shape.pos.y;

	if (height < board_top_row_filled)
	{
		board_top_row_filled = height;
	}

	for (uint8_t y = 0; y < current_shape.height; y++)
	{
		for (uint8_t x = 0; x < current_shape.width; x++)
		{
			bool filled = current_shape.val[shape_size * (y + current_shape.padding_top) + (x + current_shape.padding_left)];

			if (filled)
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
	uint8_t filled_rows_length = VECTOR_LENGTH(filled_rows_indexes.dense);
	uint8_t rows_to_move	   = 0;
	uint8_t rows_to_remove	   = 0;
	size_t	size			   = 0;

	if (filled_rows_length == 0)
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

	size = sizeof(uint8_t) * filled_rows_length * BOARD_COLS;
	memset(board + (board_top_row_filled * BOARD_COLS), 0, size);
	board_top_row_filled += filled_rows_length;

	g_score.current += filled_rows_length;

	if (g_score.current > g_score.record)
	{
		g_score.record += filled_rows_length;
	}

	if (level < c_max_level)
	{
		level = (uint8_t)ceil((float32_t)g_score.current / 10);
	}
}

static void process_game_over_filled_rows(void)
{
	game_over_filled_rows_elapsed_time += g_delta_time;

	if (game_over_filled_rows_elapsed_time >= c_game_over_filled_rows_velocity)
	{
		game_over_filled_rows_elapsed_time = 0;
		game_over_filled_rows++;
	}
}

static void process_prev_shape_animation(void)
{
	if (prev_shape.width == 0)
	{
		return;
	}

	prev_shape_elapsed_time += g_delta_time;

	if (prev_shape_elapsed_time >= c_prev_shape_animation_lifetime)
	{
		prev_shape.width  = 0;
		prev_shape.height = 0;
	}
}

static void set_next_shape(void)
{
	next_shape.type = rand() % SHAPES_COUNT;
	uint8_t size	= c_shape_size[next_shape.type];

	memset(next_shape.val, 0, sizeof(uint8_t) * (SHAPE_MAX_SIZE * SHAPE_MAX_SIZE));
	memcpy(next_shape.val, c_shape_list[next_shape.type], size * size);
	set_shape_padding(&next_shape);

	next_shape.pos.x = (BOARD_COLS - (c_win_padding * 2)) * 0.5 - next_shape.width * 0.5 + next_shape.padding_left;
	next_shape.pos.y = c_win_next_shape_height * 0.5 - next_shape.height * 0.5 - next_shape.padding_top - c_win_padding;
}

static void set_prev_shape(void)
{
	uint8_t size = c_shape_size[current_shape.type];

	prev_shape.type			  = current_shape.type;
	prev_shape.pos			  = current_shape.pos;
	prev_shape.width		  = current_shape.width;
	prev_shape.height		  = current_shape.height;
	prev_shape.padding_left	  = current_shape.padding_left;
	prev_shape.padding_right  = current_shape.padding_right;
	prev_shape.padding_top	  = current_shape.padding_top;
	prev_shape.padding_bottom = current_shape.padding_bottom;

	memset(prev_shape.val, 0, sizeof(uint8_t) * (SHAPE_MAX_SIZE * SHAPE_MAX_SIZE));
	memcpy(prev_shape.val, current_shape.val, size * size);
	prev_shape_elapsed_time = 0;
}

static void set_current_shape(void)
{
	uint8_t size = c_shape_size[next_shape.type];

	current_shape.type = next_shape.type;
	memset(current_shape.val, 0, sizeof(uint8_t) * (SHAPE_MAX_SIZE * SHAPE_MAX_SIZE));
	memcpy(current_shape.val, c_shape_list[current_shape.type], size * size);
	set_shape_padding(&current_shape);

	current_shape.pos.x = floor(BOARD_COLS * 0.5 - current_shape.width * 0.5 + current_shape.padding_left);
	current_shape.pos.y = 0;
}

static void save_score(void)
{
	if (g_score.current < g_score.record)
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
	if (g_score.current_label < g_score.current)
	{
		g_score.current_label += c_score_velocity * g_delta_time;

		if (g_score.current_label > g_score.current)
		{
			g_score.current_label = g_score.current;
		}
	}

	if (g_score.record_label < g_score.record)
	{
		g_score.record_label += c_score_velocity * g_delta_time;

		if (g_score.record_label > g_score.record)
		{
			g_score.record_label = g_score.record;
		}
	}
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
	char		level_count[3] = { '\0' };
	const char *title		   = "NEXT";
	const char *lines_label	   = "Level:";

	uint8_t padding_x = c_win_padding * 2,
			padding_y = c_win_next_shape_height - 2;

	sprintf(level_count, "%d", level);

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
	mvwprintw(win_next_shape, padding_y, strlen(lines_label) + padding_x + 1, "%s", level_count);
	wattroff(win_next_shape, COLOR_PAIR(COLOR_PAIR_GREEN_DEFAULT));
	// shape
	render_shape(win_next_shape, &next_shape, false);

	wrefresh(win_next_shape);
}

static void render_win_score()
{
	wclear(win_score);
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

static void render_win_paused(void)
{
	const char *title = "Paused";

	wclear(win_paused);
	wattron(win_paused, COLOR_PAIR(COLOR_PAIR_RED_MEDIUM));
	box(win_paused, 0, 0);
	wattroff(win_paused, COLOR_PAIR(COLOR_PAIR_RED_MEDIUM));

	mvwprintw(win_paused, floor(c_win_paused_height * 0.5), (c_win_paused_width * 0.5) - floor(strlen(title) * 0.5), "%s", title);

	wrefresh(win_paused);
	win_paused_active = true;
}

static void render_shape(WINDOW *win, shape_t *shape, bool shadow)
{
	uint8_t color	   = c_shape_colors[shape->type];
	uint8_t shape_size = c_shape_size[shape->type];

	for (uint8_t y = 0; y < shape_size; y++)
	{
		for (uint8_t x = 0; x < shape_size; x++)
		{
			bool filled = shape->val[shape_size * y + x];

			if (filled && shadow)
			{
				wattron(win, COLOR_PAIR(color * 10));
				mvwprintw(win, shape->shadow_pos_y + y + c_win_padding, (shape->pos.x * 2) + (x * 2) + c_win_padding, "[]");
				wattroff(win, COLOR_PAIR(color * 10));
			}

			if (filled)
			{
				wattron(win, COLOR_PAIR(color));
				mvwprintw(win, shape->pos.y + y + c_win_padding, (shape->pos.x * 2) + (x * 2) + c_win_padding, "[]");
				wattroff(win, COLOR_PAIR(color));
			}
		}
	}
}

static void render_board(void)
{
	uint8_t color			= 0;
	uint8_t prev_shape_size = c_shape_size[prev_shape.type];

	for (int16_t y = BOARD_ROWS - 1; y >= board_top_row_filled; y--)
	{
		for (uint8_t x = 0; x < BOARD_COLS; x++)
		{
			// white rows for game over animation
			if (board_top_row_filled == 0 && game_over_filled_rows >= (BOARD_ROWS - y))
			{
				wattron(win_board, COLOR_PAIR(COLOR_PAIR_WHITE_HIGH));
				mvwaddch(win_board, y + c_win_padding, (x * 2) + c_win_padding, CH_SHAPE_FILL);
				mvwaddch(win_board, y + c_win_padding, (x * 2) + 1 + c_win_padding, CH_SHAPE_FILL);
				wattroff(win_board, COLOR_PAIR(COLOR_PAIR_WHITE_HIGH));
			}
			else
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
				// highlight animation for last shape
				else if (prev_shape.width > 0 &&
						 (x >= (prev_shape.pos.x + prev_shape.padding_left)) &&
						 (x < (prev_shape.pos.x + prev_shape.padding_left + prev_shape.width)) &&
						 (y >= (prev_shape.pos.y + prev_shape.padding_top)) &&
						 (y < (prev_shape.pos.y + prev_shape.padding_top + prev_shape.height)) &&
						 prev_shape.val[prev_shape_size * (uint8_t)(y - prev_shape.pos.y) + (uint8_t)(x - prev_shape.pos.x)])
				{
					color = (color * 10) + ((uint8_t)((prev_shape_elapsed_time) * 10) % 3);
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
}