#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include <allegro.h>
#include <pthread.h>
#include <stdio.h>
#include <allegro/keyboard.h>

#include "train.h"
#include "conf/multimedia.h"
#include "rt_thread.h"

#endif

extern bool terminate;

void start_graphics(void);
void *graphic_behaviour();
void draw_all_cells(void);
void draw_cell(int, int);
void draw_all_trains(void);
void update_all_cells(void);
void update_cell(int, int);
void stop_graphics(void);
void lw_allocate(BITMAP **const);
void *keyboard_behaviour(void *const);
int start_keyboard(void);
void stop_keyboard(void);
