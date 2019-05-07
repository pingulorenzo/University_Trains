#ifndef TRAIN_H
#define TRAIN_H

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <allegro/keyboard.h>

#include "conf/train.h"
#include "rt_thread.h"

typedef enum status {ARRIVING, LEAVING, DOCKING, DOCKED} status;
typedef enum last_p {UP, RIGHT, DOWN, LEFT} last_p;	// It's last position compared to the current one, not last move type.
typedef enum next_p {UP_N, RIGHT_N, DOWN_N, LEFT_N} next_p;

typedef struct position {
	int 				x;						// train x offset
	int 				y;						// train y offset
	last_p			last;					// previous cell position compared to the current one
	next_p			next;
} position;

typedef struct wagon {
	unsigned int       	id;							// wagon[] index
	position 		       	pos;						// train position
	bool								on_cell;				// is the wagon currently in the center of a cell?
} wagon;

typedef struct train {
	bool 			         	alive;					// is train allocated? is the data meaningful?
	unsigned int       	tid;						// thread identifier
	unsigned int       	id;							// trains[] index
	position 		       	pos;						// train position
	wagon								wagons[2];			// array for wagon descriptors
	bool 		       			right;					// is the train going right?
	bool								on_cell;				// is the train currently in the center of a cell?
	bool								on_dock;				// is the train on dock?
	int									dock;						// assigned dock
	int									my_sec;					// array index of train's sectorò
	status             	status;					// what is the train doing?
	unsigned int 				arrival;				// seconds since 00:00 representing the time of arrival in station (spawn)
	unsigned int 				departure;			// seconds since 00:00 representing the time of desired departure from station;
	unsigned int				prio;						// priority value from 1 to 5 (higher / greater)
	pthread_mutex_t 	 	mtx;						// mutex to protect the struct
} train;

extern train trains[N_MAX];
extern uint8_t n_trains;
extern unsigned int time_g;
extern FILE *file;
extern pthread_mutex_t trains_mtx;

void init_train(train *const, int, int, bool);
void init_my_train(train *const, int, int, bool, int);
void *train_routine(void *const);
void init_train_manager(void);
void *time_routine(void *const);
void open_train_schedule(void);
void close_train_schedule(void);
int load_train(void);
int load_my_train(bool);
void flush_trains(void);
void move_loco(train *const);
void move_wagon(wagon *const);
void kill_train(int);
bool get_on_cell(int, int);
void read_time(char *const, char *const);
void read_prio(char *const);
void read_right(char *const);
int get_time(void);
bool prio_permit(train *const);
void assign_dock(train *const);
void set_in_switches(int, bool);
void set_out_switches(int, bool);
void set_in_lights(int, bool, bool);
void set_out_lights(int, bool, bool);
int get_x_by_dock(int);

#endif
