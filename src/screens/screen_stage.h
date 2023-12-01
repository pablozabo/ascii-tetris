#ifndef SCREEN_STAGE_H
#define SCREEN_STAGE_H

#include "../defs.h"

void screen_stage_init(void);
void screen_stage_dispose(void);
bool screen_stage_is_completed(void);
void screen_stage_update(void);
void screen_stage_render(void);
void screen_stage_window_resized(void);

#endif