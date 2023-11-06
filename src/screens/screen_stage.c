#include "screen_stage.h"
#include "../common.h"
#include "../data_structures/data_structures.h"
#include "../shapes.h"

extern int		 g_key;
extern score_t	 g_score;
extern float32_t g_delta_time;

#define BOARD_ROWS 20
#define BOARD_COLS 10

static const uint8_t c_win_board_width		 = 22;
static const uint8_t c_win_board_height		 = 22;
static const uint8_t c_win_next_shape_width	 = 20;
static const uint8_t c_win_next_shape_height = 11;
static const uint8_t c_win_score_width		 = 20;
static const uint8_t c_win_score_height		 = 11;

static const uint8_t  c_win_padding	   = 1;
static const uint32_t c_score_velocity = 30;

static WINDOW *win_board;
static WINDOW *win_next_shape;
static WINDOW *win_score;

shape_t next_shape;
shape_t current_shape;
uint8_t board[BOARD_ROWS * BOARD_COLS];

// init
static void
create_windows(void);
// update
static void handle_input(void);
static void save_score(void);
static void update_score_labels(void);
static void set_next_shape(void);
// render
static void render_win_board(void);
static void render_win_next_shape(void);
static void render_win_score(void);
static void render_shape(WINDOW *win, shape_t *shape);

void screen_stage_init(void)
{
	g_score.record_label = g_score.record;

	srand(time(NULL));
	create_windows();
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
	handle_input();
	update_score_labels();
}

void screen_stage_render(void)
{
	render_win_next_shape();
	render_win_score();

	wclear(win_board);
	render_win_board();
	// TODO: draw shapes in board
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
	// player.direction = PLAYER_DIRECTION_IDLE;

	// if (g_key > 0)
	// {
	// 	if (g_key == KEY_LEFT)
	// 	{
	// 		player.direction = PLAYER_DIRECTION_LEFT;
	// 	}
	// 	else if (g_key == KEY_RIGHT)
	// 	{
	// 		player.direction = PLAYER_DIRECTION_RIGHT;
	// 	}
	// 	else if (g_key == CH_SPACE && player.ball_attached)
	// 	{
	// 		player.ball_attached->direction.x = 1;
	// 		player.ball_attached->direction.y = -1;
	// 		vec2_normalize_to(&player.ball_attached->direction);
	// 		player.ball_attached = NULL;
	// 	}
	// }
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

static void set_next_shape(void)
{
	next_shape.type	 = SHAPE_TYPE_T;
	next_shape.pos.x = c_win_next_shape_width * 0.5 - SHAPE_COLS * 0.5 - c_win_padding;
	next_shape.pos.y = c_win_next_shape_height * 0.5 - SHAPE_ROWS * 0.5;
	memcpy(next_shape.val, c_shape_list[next_shape.type], SHAPE_ROWS * SHAPE_COLS);
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
				mvwprintw(win, shape->pos.y + y, shape->pos.x + (x * 2), "[]");
			}
		}
	}

	wattroff(win, COLOR_PAIR(color));
}