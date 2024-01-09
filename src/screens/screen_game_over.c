#include "screen_game_over.h"
#include "../common.h"

extern int		 g_key;
extern float32_t g_delta_time;
extern char		*g_asset_game_over;
extern score_t	 g_score;

static const uint8_t  c_win_game_over_width		   = 51;
static const uint8_t  c_win_game_over_height	   = 6;
static const uint8_t  c_win_new_record_width	   = 51;
static const uint8_t  c_win_new_record_height	   = 3;
static const uint8_t  c_win_play_again_width	   = 51;
static const uint8_t  c_win_play_again_height	   = 3;
static const uint32_t c_record_points_acceleration = 1;

static WINDOW *win_game_over;
static WINDOW *win_new_record;
static WINDOW *win_play_again;

static bool		 key_enter_pressed		 = false;
static bool		 render_play_again_label = true;
static float32_t elapsed_time			 = 0;
static uint32_t	 record_points			 = 0;
static uint32_t	 record_points_velocity	 = 0;

static void render_game_over(void);
static void render_new_record(void);
static void render_play_again(void);

void screen_game_over_init(void)
{
	uint8_t offset_y, offset_x;

	set_offset_yx(c_win_game_over_height + c_win_new_record_height + c_win_play_again_height, c_win_game_over_width, &offset_y, &offset_x);
	win_game_over = newwin(c_win_game_over_height, c_win_game_over_width, offset_y, offset_x);
	scrollok(win_game_over, TRUE);

	win_new_record = newwin(c_win_new_record_height, c_win_new_record_width, offset_y + c_win_game_over_height, offset_x);
	scrollok(win_new_record, TRUE);

	win_play_again = newwin(c_win_play_again_height, c_win_play_again_width, offset_y + c_win_game_over_height + c_win_new_record_height, offset_x);
	scrollok(win_play_again, TRUE);

	key_enter_pressed	   = false;
	elapsed_time		   = 0;
	record_points		   = 0;
	record_points_velocity = 1;

	render_game_over();
}

void screen_game_over_dispose(void)
{
	wclear(win_game_over);
	wrefresh(win_game_over);
	delwin(win_game_over);

	wclear(win_new_record);
	wrefresh(win_new_record);
	delwin(win_new_record);

	wclear(win_play_again);
	wrefresh(win_play_again);
	delwin(win_play_again);
}

bool screen_game_over_is_completed(void)
{
	return key_enter_pressed;
}

void screen_game_over_update(void)
{
	elapsed_time += g_delta_time;
	key_enter_pressed		= key_enter_pressed || g_key == CH_ENTER;
	render_play_again_label = (uint32_t)(elapsed_time) % 2;

	if (g_score.current >= g_score.record && record_points < g_score.current)
	{
		record_points_velocity += c_record_points_acceleration;
		record_points += record_points_velocity;

		if (record_points > g_score.current)
		{
			record_points = g_score.current;
		}
	}
}

void screen_game_over_render(void)
{
	if (g_score.current >= g_score.record && record_points <= g_score.current)
	{
		render_new_record();
	}

	render_play_again();
}

void screen_game_over_window_resized(void)
{
	render_game_over();
}

static void render_game_over(void)
{
	uint32_t i = 0;
	uint8_t	 ch,
		x = 0,
		y = 0;

	werase(win_game_over);
	wattron(win_game_over, COLOR_PAIR(COLOR_PAIR_RED_DEFAULT));

	while ((ch = g_asset_game_over[i++]) != CH_EOS)
	{
		if (ch == CH_EOL)
		{
			x = 0;
			y++;
			continue;
		}

		mvwprintw(win_game_over, y, x++, "%c", ch);
	}

	wattroff(win_game_over, COLOR_PAIR(COLOR_PAIR_RED_DEFAULT));
	wrefresh(win_game_over);
}

static void render_new_record(void)
{
	char record[30] = { '\0' };
	wclear(win_new_record);

	sprintf(record, "New record! %d", record_points);

	wrefresh(win_new_record);
}

static void render_play_again(void)
{
	uint8_t offset_x;
	werase(win_play_again);

	if (render_play_again_label)
	{
		offset_x = (c_win_play_again_width - 25) * 0.5;
		mvwprintw(win_play_again, 2, offset_x, "Press enter to play again");
	}

	wrefresh(win_play_again);
}