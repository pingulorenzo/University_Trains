#ifndef CONF_MULTIMEDIA_H
#define CONF_MULTIMEDIA_H

#include "station.h"

#define WINDOW				600								// Window width and height
#define CELL_SIZE			(WINDOW / C_MAX) 	// Cell size


/* ========================  *
*  ====== GUI TASKS =======  *
*  ======================== */

#define PRD_GRAPHICS		20	// Task period
#define DL_GRAPHICS 		20	// Task deadline
#define PRIO_GRAPHICS 	50	// Task priority
#define PRD_KEYBOARD		100	// Task period
#define DL_KEYBOARD 		100	// Task deadline
#define PRIO_KEYBOARD 	10	// Task priority

#endif
