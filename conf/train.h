#ifndef CONF_TRAIN_H
#define CONF_TRAIN_H

/* ========================
*  ======== TRAINS ========
*  ======================== */

#define	N_MAX								256						// Maximum number of trains
#define	N_SECTION						4							// Number of station sections
#define	DUMMY								8							// dummy char range
#define STARTING_TIME				0							// Starting time of simulation
#define TIME_STEP						1							// Time step size
#define MOVE_STEP						2							// Move step size
#define LEAVE_TIME					250						// Steps to leave the station
#define TIME_MAX						60 * 60 * 24	// MOD size
#define N_PLATFORM					8							// Platform number

/* ========================
*  ===== STATE UPDATE =====
*  ======================== */

#define PRD_TRAINS			20						// Default task period
#define DL_TRAINS				20						// Default task deadline
#define PRIO_TRAINS			20						// Default task priority


#endif
