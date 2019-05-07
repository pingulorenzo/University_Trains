#ifndef STATION_H
#define STATION_H

#endif

#include <stdbool.h>
#include "conf/station.h"
#include "train.h"

typedef enum cell_t {TILE, TRACK, DOCK_TRACK, SEM} cell_t;

typedef struct cell {
	bool		up;
	bool		down;
	bool		right;
	bool		left;
	bool		green; 
	cell_t	content;
} cell;

extern cell cells[R_MAX][C_MAX];
extern cell *switches[3 * N_SECTION];
extern cell *lights[N_PLATFORM][2];

void init_station(void);
void init_docks(void);
void init_docks_links(void);
void init_entrances(void);
void init_switches(void);
void init_mid_tracks(void);
void init_lights(void);
void assign_switches(void);
