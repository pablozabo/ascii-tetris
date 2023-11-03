#ifndef SCREEN_GAME_OVER_H
#define SCREEN_GAME_OVER_H

#include "../defs.h"

void screen_game_over_init(void);
void screen_game_over_dispose(void);
bool screen_game_over_is_completed(void);
void screen_game_over_update(void);
void screen_game_over_render(void);
void screen_game_over_window_resized(void);

#endif