#include "common.h"
#include "defs.h"
#include "screens/screens.h"

#define TERMINAL_COLS 100
#define TERMINAL_ROWS 50

#define FILE_SPLASH "assets/splash.txt"
#define FILE_GAME_OVER "assets/game_over.txt"

typedef enum screen_t
{
	SCREEN_INIT		 = 1,
	SCREEN_STAGE	 = 2,
	SCREEN_GAME_OVER = 3
} screen_t;

// #GLOBAL VARIABLES
bool	  g_running = true;
int		  g_key;
float32_t g_delta_time		= 0;
char	 *g_asset_splash	= NULL;
char	 *g_asset_game_over = NULL;
score_t	  g_score			= { .current = 0 };

static const float32_t c_target_frame_time = 1000 / 30; // 30 FPS

static screen_action_t		 screen_action_init			  = NULL;
static screen_action_t		 screen_action_dispose		  = NULL;
static screen_action_t		 screen_action_update		  = NULL;
static screen_action_t		 screen_action_render		  = NULL;
static screen_is_completed_t screen_is_completed		  = NULL;
static screen_action_t		 screen_action_window_resized = NULL;
static screen_t				 current_screen				  = 0;

static float32_t last_update_time = 0.0;

static void init(void);
static void dispose(void);
static void load_assets(void);
static void load_asset(const char *file, char **dest);
static void load_score(void);
static void update_state(void);
static void loop(void);

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	init();
	loop();
	dispose();

	return 0;
}

static void init(void)
{
	load_assets();
	load_score();
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	// timeout(10);
	resize_term(TERMINAL_ROWS, TERMINAL_COLS);
	start_color();

	init_color(CUSTOM_COLOR_RED_DEFAULT, 900, 0, 0);
	init_color(CUSTOM_COLOR_RED_LOW, 300, 0, 0);
	init_color(CUSTOM_COLOR_RED_MEDIUM, 500, 0, 0);
	init_color(CUSTOM_COLOR_RED_HIGH, 1000, 0, 0);

	init_color(CUSTOM_COLOR_BLUE_DEFAULT, 0, 0, 900);
	init_color(CUSTOM_COLOR_BLUE_LOW, 0, 0, 300);
	init_color(CUSTOM_COLOR_BLUE_MEDIUM, 0, 0, 500);
	init_color(CUSTOM_COLOR_BLUE_HIGH, 0, 0, 1000);

	init_color(CUSTOM_COLOR_GREEN_DEFAULT, 0, 900, 0);
	init_color(CUSTOM_COLOR_GREEN_LOW, 0, 300, 0);
	init_color(CUSTOM_COLOR_GREEN_MEDIUM, 0, 500, 0);
	init_color(CUSTOM_COLOR_GREEN_HIGH, 0, 1000, 0);

	init_color(CUSTOM_COLOR_YELLOW_DEFAULT, 900, 900, 0);
	init_color(CUSTOM_COLOR_YELLOW_LOW, 300, 300, 0);
	init_color(CUSTOM_COLOR_YELLOW_MEDIUM, 500, 500, 0);
	init_color(CUSTOM_COLOR_YELLOW_HIGH, 1000, 1000, 0);

	init_color(CUSTOM_COLOR_ORANGE_DEFAULT, 900, 450, 0);
	init_color(CUSTOM_COLOR_ORANGE_LOW, 300, 150, 0);
	init_color(CUSTOM_COLOR_ORANGE_MEDIUM, 500, 250, 0);
	init_color(CUSTOM_COLOR_ORANGE_HIGH, 1000, 500, 0);

	init_color(CUSTOM_COLOR_CYAN_DEFAULT, 0, 900, 900);
	init_color(CUSTOM_COLOR_CYAN_LOW, 0, 300, 300);
	init_color(CUSTOM_COLOR_CYAN_MEDIUM, 0, 500, 500);
	init_color(CUSTOM_COLOR_CYAN_HIGH, 0, 1000, 1000);

	init_color(CUSTOM_COLOR_MAGENTA_DEFAULT, 900, 0, 900);
	init_color(CUSTOM_COLOR_MAGENTA_LOW, 300, 0, 300);
	init_color(CUSTOM_COLOR_MAGENTA_MEDIUM, 500, 0, 500);
	init_color(CUSTOM_COLOR_MAGENTA_HIGH, 1000, 0, 1000);

	init_color(CUSTOM_COLOR_WHITE_DEFAULT, 900, 900, 900);
	init_color(CUSTOM_COLOR_WHITE_LOW, 300, 300, 300);
	init_color(CUSTOM_COLOR_WHITE_MEDIUM, 500, 500, 500);
	init_color(CUSTOM_COLOR_WHITE_HIGH, 1000, 1000, 1000);

	init_pair(COLOR_PAIR_RED_DEFAULT, CUSTOM_COLOR_RED_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_RED_LOW, CUSTOM_COLOR_RED_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_RED_MEDIUM, CUSTOM_COLOR_RED_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_RED_HIGH, CUSTOM_COLOR_RED_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_GREEN_DEFAULT, CUSTOM_COLOR_GREEN_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_GREEN_LOW, CUSTOM_COLOR_GREEN_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_GREEN_MEDIUM, CUSTOM_COLOR_GREEN_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_GREEN_HIGH, CUSTOM_COLOR_GREEN_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_BLUE_DEFAULT, CUSTOM_COLOR_BLUE_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_BLUE_LOW, CUSTOM_COLOR_BLUE_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_BLUE_MEDIUM, CUSTOM_COLOR_BLUE_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_BLUE_HIGH, CUSTOM_COLOR_BLUE_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_YELLOW_DEFAULT, CUSTOM_COLOR_YELLOW_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_YELLOW_LOW, CUSTOM_COLOR_YELLOW_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_YELLOW_MEDIUM, CUSTOM_COLOR_YELLOW_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_YELLOW_HIGH, CUSTOM_COLOR_YELLOW_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_ORANGE_DEFAULT, CUSTOM_COLOR_ORANGE_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_ORANGE_LOW, CUSTOM_COLOR_ORANGE_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_ORANGE_MEDIUM, CUSTOM_COLOR_ORANGE_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_ORANGE_HIGH, CUSTOM_COLOR_ORANGE_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_CYAN_DEFAULT, CUSTOM_COLOR_CYAN_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_CYAN_LOW, CUSTOM_COLOR_CYAN_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_CYAN_MEDIUM, CUSTOM_COLOR_CYAN_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_CYAN_HIGH, CUSTOM_COLOR_CYAN_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_MAGENTA_DEFAULT, CUSTOM_COLOR_MAGENTA_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_MAGENTA_LOW, CUSTOM_COLOR_MAGENTA_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_MAGENTA_MEDIUM, CUSTOM_COLOR_MAGENTA_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_MAGENTA_HIGH, CUSTOM_COLOR_MAGENTA_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_WHITE_DEFAULT, CUSTOM_COLOR_WHITE_DEFAULT, COLOR_BLACK);
	init_pair(COLOR_PAIR_WHITE_LOW, CUSTOM_COLOR_WHITE_LOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_WHITE_MEDIUM, CUSTOM_COLOR_WHITE_MEDIUM, COLOR_BLACK);
	init_pair(COLOR_PAIR_WHITE_HIGH, CUSTOM_COLOR_WHITE_HIGH, COLOR_BLACK);

	init_pair(COLOR_PAIR_BLUE_BK, CUSTOM_COLOR_WHITE_DEFAULT, CUSTOM_COLOR_BLUE_DEFAULT);
	init_pair(COLOR_PAIR_RED_BK, CUSTOM_COLOR_WHITE_DEFAULT, CUSTOM_COLOR_RED_DEFAULT);

	refresh();
}

static void dispose(void)
{
	if (g_asset_splash)
	{
		free(g_asset_splash);
	}

	if (g_asset_game_over)
	{
		free(g_asset_game_over);
	}

	use_default_colors();
	endwin();
}

static void loop(void)
{
	while (g_running)
	{
		g_key = getch();

		if (g_key == KEY_F(1) || g_key == CH_ESC)
		{
			break;
		}
		else if (g_key == KEY_RESIZE)
		{
			resize_term(TERMINAL_ROWS, TERMINAL_COLS);
			noecho();
			cbreak();
			curs_set(0);
			refresh();

			if (screen_action_window_resized)
			{
				screen_action_window_resized();
			}
		}

		float32_t real_delta_time = CURRENT_TIME - last_update_time;
		last_update_time += real_delta_time;

		if (real_delta_time < c_target_frame_time)
		{
			napms(c_target_frame_time - real_delta_time);
		}

		g_delta_time = real_delta_time / 1000;

		update_state();
		screen_action_update();
		screen_action_render();
	}

	if (screen_action_dispose)
	{
		screen_action_dispose();
	}
}

static void update_state(void)
{
	if (!current_screen)
	{
		screen_action_init			 = &screen_init_init;
		screen_action_dispose		 = &screen_init_dispose;
		screen_action_update		 = &screen_init_update;
		screen_action_render		 = &screen_init_render;
		screen_is_completed			 = &screen_init_is_completed;
		screen_action_window_resized = NULL;
		screen_action_init();
		current_screen = SCREEN_INIT;
	}
	else if ((current_screen == SCREEN_INIT ||
			  current_screen == SCREEN_GAME_OVER) &&
			 screen_is_completed())
	{
		screen_action_dispose();
		screen_action_init			 = &screen_stage_init;
		screen_action_dispose		 = &screen_stage_dispose;
		screen_action_update		 = &screen_stage_update;
		screen_action_render		 = &screen_stage_render;
		screen_is_completed			 = &screen_stage_is_completed;
		screen_action_window_resized = NULL;
		screen_action_init();
		current_screen = SCREEN_STAGE;
	}
	else if (current_screen == SCREEN_STAGE && screen_is_completed())
	{
		screen_action_dispose();
		screen_action_init			 = &screen_game_over_init;
		screen_action_dispose		 = &screen_game_over_dispose;
		screen_action_update		 = &screen_game_over_update;
		screen_action_render		 = &screen_game_over_render;
		screen_is_completed			 = &screen_game_over_is_completed;
		screen_action_window_resized = &screen_game_over_window_resized;
		screen_action_init();
		current_screen = SCREEN_GAME_OVER;
	}
}

static void load_assets(void)
{
	load_asset(FILE_SPLASH, &g_asset_splash);
	load_asset(FILE_GAME_OVER, &g_asset_game_over);
}

static void load_asset(const char *file, char **dest)
{
	FILE *f = fopen(file, "r");
	ASSERT(f);

	fseek(f, 0, SEEK_END);
	int32_t length = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);
	*dest = calloc(length, sizeof(char));
	ASSERT(*dest);

	fread(*dest, sizeof(char), length, f);
	fclose(f);
}

static void load_score(void)
{
	FILE *f = fopen(FILE_SCORE, "r");

	if (!f)
	{
		return;
	}

	fscanf(f, "%hu;", &g_score.record);
	fclose(f);
}
