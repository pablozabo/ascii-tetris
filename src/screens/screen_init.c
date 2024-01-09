#include "screen_init.h"
#include "../common.h"

#define ASSET_SPLASH_SECOND_SECTION_ROW_INDEX 4
#define ASSET_SPLASH_THIRD_SECTION_ROW_INDEX 10

extern char		*g_asset_splash;
extern int		 g_key;
extern float32_t g_delta_time;

static const char	*c_label_start			  = "Press ENTER to start";
static const uint8_t c_win_splash_width		  = 27;
static const uint8_t c_win_splash_height	  = 14;
static const uint8_t c_win_actions_width	  = 20;
static const uint8_t c_win_actions_height	  = 2;
static const uint8_t c_win_actions_margin_top = 2;

static WINDOW	*win_splash;
static WINDOW	*win_actions;
static bool		 print_label_start = true;
static bool		 key_enter_pressed = false;
static float32_t elapsed_time	   = 0;

static void render_splash(void);
static void render_actions(void);

void screen_init_init(void)
{
	uint8_t offset_y, offset_y2, offset_x;

	set_offset_yx(c_win_splash_height, c_win_splash_width, &offset_y, &offset_x);
	win_splash = newwin(c_win_splash_height, c_win_splash_width, offset_y, offset_x);
	scrollok(win_splash, TRUE);

	set_offset_yx(c_win_actions_height, c_win_actions_width, &offset_y2, &offset_x);
	win_actions = newwin(c_win_actions_height, c_win_actions_width, offset_y + c_win_splash_height + c_win_actions_margin_top, offset_x);
	scrollok(win_actions, TRUE);

	render_splash();
}

void screen_init_dispose(void)
{
	wclear(win_splash);
	wrefresh(win_splash);
	delwin(win_splash);

	wclear(win_actions);
	wrefresh(win_actions);
	delwin(win_actions);
}

bool screen_init_is_completed(void)
{
	return key_enter_pressed;
}

void screen_init_update(void)
{
	elapsed_time += g_delta_time;
	key_enter_pressed = key_enter_pressed || g_key == CH_ENTER;
	print_label_start = !key_enter_pressed && (uint32_t)(elapsed_time) % 2;
}

void screen_init_render(void)
{
	render_splash();
	render_actions();
}

static void render_splash(void)
{
	uint32_t i = 0;
	uint8_t	 ch,
		x = 0,
		y = 0;

	werase(win_splash);
	wattron(win_splash, COLOR_PAIR(COLOR_PAIR_YELLOW_DEFAULT));

	while ((ch = g_asset_splash[i++]) != CH_EOS)
	{
		if (ch == CH_EOL)
		{
			x = 0;
			y++;
			continue;
		}

		if (y == ASSET_SPLASH_SECOND_SECTION_ROW_INDEX)
		{
			wattroff(win_splash, COLOR_PAIR(COLOR_PAIR_YELLOW_DEFAULT));
			wattron(win_splash, COLOR_PAIR(COLOR_PAIR_RED_DEFAULT));
		}
		else if (y == ASSET_SPLASH_THIRD_SECTION_ROW_INDEX)
		{
			wattroff(win_splash, COLOR_PAIR(COLOR_PAIR_RED_DEFAULT));
			wattron(win_splash, COLOR_PAIR(COLOR_PAIR_GREEN_DEFAULT));
		}

		mvwprintw(win_splash, y, x++, "%c", ch);
	}

	wattroff(win_splash, COLOR_PAIR(COLOR_PAIR_RED_DEFAULT));
	wrefresh(win_splash);
}

void render_actions(void)
{
	wclear(win_actions);

	if (print_label_start)
	{
		mvwprintw(win_actions, 0, 0, "%s", c_label_start);
	}

	wrefresh(win_actions);
}