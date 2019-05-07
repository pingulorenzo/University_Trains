#include "train.h"
#include "multimedia.h"
#include "station.h"

unsigned int time_g;																		// global time variable
bool busy_sec[N_SECTION];																// array of section busy status
bool busy_dock[N_PLATFORM];															// array of platform busy status
FILE *file;																							// file pointer
train trains[N_MAX];																		// container for trains' descriptors
uint8_t n_trains = 0; 																	// current number of trains in the station
pthread_mutex_t trains_mtx = PTHREAD_MUTEX_INITIALIZER; // mutex to protect trains

/* Initialazes train manager variables and data structures */
void init_train_manager() {
	int tid;

	pthread_mutex_lock(&trains_mtx);

	open_train_schedule();
	time_g = STARTING_TIME;

	for (int i = 0; i < N_MAX; ++i) {
		pthread_mutex_init(&trains[i].mtx, NULL);
		trains[i].alive = false;
	}

	pthread_mutex_unlock(&trains_mtx);
}

void open_train_schedule() {
	file = fopen("./conf/sched.txt", "r");
}

/* Gracefully ends train manager */
void close_train_schedule() {
	fclose(file);
}

void flush_trains() {
	for (int i = 0; i < N_MAX; i++) {
		trains[i].alive = false;
	}

	close_train_schedule();
	open_train_schedule();

	time_g = STARTING_TIME;
}

/* Loads a train with direction, priority and arrival/departure time from file to trains[].
 * returns train's trains[] array index */
int load_train() {
	int ret, tid, id;
	char r[2], line[2], dummy[2];

	pthread_mutex_lock(&trains_mtx);

	for (int i = 0; i < N_MAX; i++) {

		if (!trains[i].alive) {

			fgets(line, 2, file);
			fgets(dummy, DUMMY, file);

			if (line[0] == '/') {
				pthread_mutex_unlock(&trains_mtx);
				return N_MAX;
			}

			n_trains++;

			tid = start_thread(train_routine, &trains[i], SCHED_FIFO, PRD_TRAINS, DL_TRAINS, PRIO_TRAINS);
			if (tid < 0) {
				trains[i].alive = false;
				printf("ABORT TRAIN LOAD\n");
				pthread_mutex_unlock(&trains_mtx);
				return -1;
			} else {
				ret = i;
				id = i;
				read_right(r);
				init_train(&trains[i], tid, id, r[0]);
			}
			break;
		}
	}

	pthread_mutex_unlock(&trains_mtx);

	return ret;
}

int load_my_train(bool dir) {
	int ret, tid, id;
	ret = N_MAX;

	pthread_mutex_lock(&trains_mtx);

	for (int i = 0; i < N_MAX; i++) {

		if (!trains[i].alive) {

			n_trains++;

			tid = start_thread(train_routine, &trains[i], SCHED_FIFO, PRD_TRAINS, DL_TRAINS, PRIO_TRAINS);
			if (tid < 0) {
				trains[i].alive = false;
				printf("ABORT MY TRAIN LOAD\n");
				pthread_mutex_unlock(&trains_mtx);
				return -1;
			} else {
				ret = i;
				id = i;
				init_my_train(&trains[i], tid, id, dir, time_g);
			}
			break;
		}
	}

	pthread_mutex_unlock(&trains_mtx);

	return ret;
}

/* Initialize the descriptor of a train right after load */
void init_train(train *const tr, int tid, int id, bool right_direction) {
	char prio[2];

	tr->alive = true;
	tr->tid = (unsigned int)tid;
	tr->id = (unsigned int)id;
	tr->right = right_direction;
	tr->pos.last = (right_direction) ? LEFT : RIGHT;
	tr->pos.next = (!right_direction) ? LEFT : RIGHT;
	tr->wagons[0].pos.last = tr->pos.last;
	tr->wagons[0].pos.next = tr->pos.next;
	tr->wagons[1].pos.last = tr->pos.last;
	tr->wagons[1].pos.next = tr->pos.next;
	tr->dock = -1;
	tr->status = ARRIVING;
	tr->arrival = get_time();
	tr->departure = get_time();
	read_prio(prio);
	tr->prio = prio[0] - '0';
}

/* Initialize the descriptor of a train right after load */
void init_my_train(train *const tr, int tid, int id, bool right_direction, int gt) {
	tr->alive = true;
	tr->tid = (unsigned int)tid;
	tr->id = (unsigned int)id;
	tr->right = right_direction;
	tr->pos.last = (right_direction) ? LEFT : RIGHT;
	tr->pos.next = (!right_direction) ? LEFT : RIGHT;
	tr->wagons[0].pos.last = tr->pos.last;
	tr->wagons[0].pos.next = tr->pos.next;
	tr->wagons[1].pos.last = tr->pos.last;
	tr->wagons[1].pos.next = tr->pos.next;
	tr->dock = -1;
	tr->status = ARRIVING;
	tr->arrival = gt + 30;
	tr->departure = tr->arrival + 300;
	tr->prio = 5;
}

/* Gracefully terminates train thread */
void kill_train(int id) {
	stop_thread(trains[id].tid);
}

/* Reads time from file */
void read_time(char h[], char m[]) {
	char dummy[DUMMY];

	fgets(h, 3, file);
	fgets(m, 3, file);
	fgets(dummy, DUMMY, file);
}

/* Reads prio from file */
void read_prio(char pr[]){
	char dummy[DUMMY];

	fgets(pr, 2, file);
	fgets(dummy, DUMMY, file);
}

/* Reads direction from file and casts it to bool (int) */
void read_right(char r[]){
	char dummy[DUMMY];

	fgets(r, 2, file);
	fgets(dummy, DUMMY, file);

	if (r[0] == 'L'){
		r[0] = 0;
	} else {
		r[0] = 1;
	}
}

/* This function transforms time written in hours and minutes to seconds past midnight*/
int get_time() {
	char h[3];
	char m[3];
	int n, k;

	read_time(h, m);

	n = 0;

	for (int i = 0; i < 2; i++) {
    n = n * 10 + (h[i] - '0');
  }

	k = n * 60; // convert to minutes
	n = 0;

	for (int i = 0; i < 2; i++) {
    n = n * 10 + (m[i] - '0');
  }

	k = (k + n) * 60; // convert to seconds

	return k;
}

/* Train thread routine function, describes the behaviour of the train during
 * his life time */
void *train_routine(void *arg) {
	train *const tr = (train *)arg;

	pthread_mutex_lock(&trains_mtx);

	switch (tr->status) {
		case ARRIVING:
			if (tr->arrival <= time_g) {
				if (tr->dock == -1) {
					assign_dock(tr);
				}
				if (!busy_sec[tr->my_sec] & tr->dock != -1) {
					tr->pos.x = get_x_by_dock(tr->dock);
					tr->pos.y = (tr->right)? 0 : WINDOW;
					tr->wagons[0].pos.x = tr->pos.x;
					tr->wagons[0].pos.y = (tr->right)? (tr->pos.y - CELL_SIZE) : (tr->pos.y + CELL_SIZE);
					tr->wagons[1].pos.x = tr->pos.x;
					tr->wagons[1].pos.y = (tr->right)? (tr->pos.y - 2 * CELL_SIZE) : (tr->pos.y + 2 * CELL_SIZE);
					busy_sec[tr->my_sec] = true;
					tr->status = DOCKING;
					set_in_switches(tr->dock, tr->right);
					set_in_lights(tr->dock, tr->right, true);
				}
			}
			break;
		case DOCKING:
			if (((tr->pos.y == DOCK_AREA_ABS - CELL_SIZE) & (!tr->right)) | ((tr->pos.y == DOCK_AREA_ABS + CELL_SIZE) & (tr->right))) {
				tr->on_dock = true;
				busy_sec[tr->my_sec] = false;
				set_in_lights(tr->dock, tr->right, false);
				switch (tr->my_sec) {
					case 0:
						tr->my_sec = 3;
						break;
					case 1:
						tr->my_sec = 2;
						break;
					case 2:
						tr->my_sec = 1;
						break;
					case 3:
						tr->my_sec = 0;
						break;
					default:
						printf("ERROR DOCKING CASE\n");
				}
				tr->status = DOCKED;
			} else {
				move_loco(tr);
				move_wagon(&tr->wagons[0]);
				move_wagon(&tr->wagons[1]);
			}
			break;
		case DOCKED:
			if (tr->departure <= time_g) {
				if (prio_permit(tr) & !busy_sec[tr->my_sec]) {
					busy_sec[tr->my_sec] = true;
					busy_dock[tr->dock - 1] = false;
					tr->status = LEAVING;
					set_out_switches(tr->dock, tr->right);
					set_out_lights(tr->dock, tr->right, true);
				}
			}
			break;
		case LEAVING:
			if (tr->pos.y == 0 - 2 * CELL_SIZE | tr->pos.y == WINDOW + 2 * CELL_SIZE) {
				set_out_lights(tr->dock, tr->right, false);
				tr->alive = false;
				busy_sec[tr->my_sec] = false;
				n_trains--;
				pthread_mutex_unlock(&trains_mtx);
				kill_train(tr->id);
			} else {
				move_loco(tr);
				move_wagon(&tr->wagons[0]);
				move_wagon(&tr->wagons[1]);
			}
			break;
		default:
			printf("ERROR STATUS CASE\n");
	}

	pthread_mutex_unlock(&trains_mtx);
}

/* This function manages train's movement base on current and last position */
void move_loco(train *tr) {
	tr->on_cell = get_on_cell(tr->pos.x, tr->pos.y);

	if (tr->on_cell) {
		switch (tr->pos.last) {
			case UP:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].right) {
					tr->pos.y += MOVE_STEP;
					tr->pos.last = LEFT;
					tr->pos.next = RIGHT;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].down) {
					tr->pos.x += MOVE_STEP;
					tr->pos.last = UP;
					tr->pos.next = DOWN;
				} else {
					tr->pos.y -= MOVE_STEP;
					tr->pos.last = RIGHT;
					tr->pos.next = LEFT;
				}
				break;
			case RIGHT:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].down) {
					tr->pos.x += MOVE_STEP;
					tr->pos.last = UP;
					tr->pos.next = DOWN;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].left) {
					tr->pos.y -= MOVE_STEP;
					tr->pos.last = RIGHT;
					tr->pos.next = LEFT;
				} else {
					tr->pos.x -= MOVE_STEP;
					tr->pos.last = DOWN;
					tr->pos.next = UP;
				}
				break;
			case DOWN:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].left) {
					tr->pos.y -= MOVE_STEP;
					tr->pos.last = RIGHT;
					tr->pos.next = LEFT;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].up) {
					tr->pos.x -= MOVE_STEP;
					tr->pos.last = DOWN;
					tr->pos.next = UP;
				} else {
					tr->pos.y += MOVE_STEP;
					tr->pos.last = LEFT;
					tr->pos.next = RIGHT;
				}
				break;
			case LEFT:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].up) {
					tr->pos.x -= MOVE_STEP;
					tr->pos.last = DOWN;
					tr->pos.next = UP;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].right) {
					tr->pos.y += MOVE_STEP;
					tr->pos.last = LEFT;
					tr->pos.next = RIGHT;
				} else {
					tr->pos.x += MOVE_STEP;
					tr->pos.last = UP;
					tr->pos.next = DOWN;
				}
				break;
			default:
				printf("ERROR MOVE TRAIN\n");
		}
	} else {
		switch (tr->pos.next) {
			case UP:
				tr->pos.x -= MOVE_STEP;
				break;
			case RIGHT:
				tr->pos.y += MOVE_STEP;
				break;
			case DOWN:
				tr->pos.x += MOVE_STEP;
				break;
			case LEFT:
				tr->pos.y -= MOVE_STEP;
				break;
			default:
				printf("ERROR MOVE TRAIN\n");
		}
	}
}

/* This function manages wagon's movement base on current and last position */
void move_wagon(wagon *tr) {
	tr->on_cell = get_on_cell(tr->pos.x, tr->pos.y);

	if (tr->on_cell) {
		switch (tr->pos.last) {
			case UP:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].right) {
					tr->pos.y += MOVE_STEP;
					tr->pos.last = LEFT;
					tr->pos.next = RIGHT;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].down) {
					tr->pos.x += MOVE_STEP;
					tr->pos.last = UP;
					tr->pos.next = DOWN;
				} else {
					tr->pos.y -= MOVE_STEP;
					tr->pos.last = RIGHT;
					tr->pos.next = LEFT;
				}
				break;
			case RIGHT:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].down) {
					tr->pos.x += MOVE_STEP;
					tr->pos.last = UP;
					tr->pos.next = DOWN;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].left) {
					tr->pos.y -= MOVE_STEP;
					tr->pos.last = RIGHT;
					tr->pos.next = LEFT;
				} else {
					tr->pos.x -= MOVE_STEP;
					tr->pos.last = DOWN;
					tr->pos.next = UP;
				}
				break;
			case DOWN:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].left) {
					tr->pos.y -= MOVE_STEP;
					tr->pos.last = RIGHT;
					tr->pos.next = LEFT;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].up) {
					tr->pos.x -= MOVE_STEP;
					tr->pos.last = DOWN;
					tr->pos.next = UP;
				} else {
					tr->pos.y += MOVE_STEP;
					tr->pos.last = LEFT;
					tr->pos.next = RIGHT;
				}
				break;
			case LEFT:
				if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].up) {
					tr->pos.x -= MOVE_STEP;
					tr->pos.last = DOWN;
					tr->pos.next = UP;
				} else if (cells[tr->pos.x / CELL_SIZE][tr->pos.y / CELL_SIZE].right) {
					tr->pos.y += MOVE_STEP;
					tr->pos.last = LEFT;
					tr->pos.next = RIGHT;
				} else {
					tr->pos.x += MOVE_STEP;
					tr->pos.last = UP;
					tr->pos.next = DOWN;
				}
				break;
			default:
				printf("ERROR MOVE TRAIN\n");
		}
	} else {
		switch (tr->pos.next) {
			case UP:
				tr->pos.x -= MOVE_STEP;
				break;
			case RIGHT:
				tr->pos.y += MOVE_STEP;
				break;
			case DOWN:
				tr->pos.x += MOVE_STEP;
				break;
			case LEFT:
				tr->pos.y -= MOVE_STEP;
				break;
			default:
				printf("ERROR MOVE TRAIN\n");
		}
	}
}

/* Assign a dock to a train based on current section's availability and if
 * succesful locks that specific dock's section */
void assign_dock(train *tr) {
	int busy_act;
	int prio_list[N_PLATFORM] = {1, 5, 3, 7, 2, 6, 4, 8};
	tr->dock = -1;

	for (int i = 0; i < N_PLATFORM; i++) {
		if (prio_list[i] <= 4) {
			busy_act = (tr->right)? 0 : 3;
		} else {
			busy_act = (tr->right)? 1 : 2;
		}

    if (!busy_dock[prio_list[i] - 1] & !busy_sec[busy_act]) {
			busy_dock[prio_list[i] - 1] = true;
			tr->my_sec = busy_act;
			tr->dock = prio_list[i];
			break;
		}
  }

}

/* Tells if a train is on a cell of the station (false if outside or in between) */
bool get_on_cell(int x, int y){
	bool ret;
	ret = false;

	if ((x % CELL_SIZE == 0) & (y % CELL_SIZE == 0) & (y > L_ENTRANCE_ABS + CELL_SIZE) & (y < R_ENTRANCE_ABS - CELL_SIZE)){
		ret = true;
	}

	return ret;
}

/* Tells if there's a train with higher permission is leaving from tr's sector
 * before tr's leaving time */
bool prio_permit(train *tr) {
	bool ret;
	bool cond1, cond2, cond3, cond4;
	ret = true;

	for (int i = 0; i < N_MAX; i++) {
		cond1 = trains[i].alive;
		cond2 = trains[i].prio > tr->prio;
		cond3 = trains[i].departure < tr->departure + (LEAVE_TIME * TIME_STEP);
		cond4 = trains[i].my_sec == tr->my_sec;

		if (cond1 & cond2 & cond3 & cond4) {
			ret = false;
			break;
		}
	}

	return ret;
}
/* Sets the value of external switches for arriving train */
void set_in_ext_switches(int d, bool r) {
	if (r) {
		switch (d) {
			case 1: case 2:
				switches[0]->up = true;
				switches[0]->down = false;
				break;
			case 3: case 4:
				switches[0]->up = false;
				switches[0]->down = true;
				break;
			case 5: case 6:
				switches[1]->up = true;
				switches[1]->down = false;
				break;
			case 7: case 8:
				switches[1]->up = false;
				switches[1]->down = true;
				break;
			default:
				printf("ERROR IN EXT SWITCHES RIGHT: %d\n", d);
		}
	} else {
		switch (d) {
			case 1: case 2:
				switches[2]->up = true;
				switches[2]->down = false;
				break;
			case 3: case 4:
				switches[2]->up = false;
				switches[2]->down = true;
				break;
			case 5: case 6:
				switches[3]->up = true;
				switches[3]->down = false;
				break;
			case 7: case 8:
				switches[3]->up = false;
				switches[3]->down = true;
				break;
			default:
				printf("ERROR IN EXT SWITCHES LEFT: %d\n", d);
			}
	}
}

/* Sets the value of internal switches for arriving train */
void set_in_int_switches(int d, bool r) {
	if (r) {
		switch (d) {
			case 1:
				switches[4]->up = true;
				switches[4]->down = false;
				break;
			case 2:
				switches[4]->up = false;
				switches[4]->down = true;
				break;
			case 3:
				switches[5]->up = true;
				switches[5]->down = false;
				break;
			case 4:
				switches[5]->up = false;
				switches[5]->down = true;
				break;
			case 5:
				switches[6]->up = true;
				switches[6]->down = false;
				break;
			case 6:
				switches[6]->up = false;
				switches[6]->down = true;
				break;
			case 7:
				switches[7]->up = true;
				switches[7]->down = false;
				break;
			case 8:
				switches[7]->up = false;
				switches[7]->down = true;
				break;
			default:
				printf("ERROR IN INT SWITCHES RIGHT: %d\n", d);
		}
	} else {
		switch (d) {
			case 1:
				switches[8]->up = true;
				switches[8]->down = false;
				break;
			case 2:
				switches[8]->up = false;
				switches[8]->down = true;
				break;
			case 3:
				switches[9]->up = true;
				switches[9]->down = false;
				break;
			case 4:
				switches[9]->up = false;
				switches[9]->down = true;
				break;
			case 5:
				switches[10]->up = true;
				switches[10]->down = false;
				break;
			case 6:
				switches[10]->up = false;
				switches[10]->down = true;
				break;
			case 7:
				switches[11]->up = true;
				switches[11]->down = false;
				break;
			case 8:
				switches[11]->up = false;
				switches[11]->down = true;
				break;
			default:
				printf("ERROR IN INT SWITCHES LEFT: %d\n", d);
		}
	}
}

/* Sets the value of switches for arriving train */
void set_in_switches(int d, bool r) {
	set_in_ext_switches(d, r);
	set_in_int_switches(d, r);
}

/* Sets the value of external switches for departing train */
void set_out_ext_switches(int d, bool r) {
	if (r) {
		switch (d) {
			case 1: case 2:
				switches[2]->up = true;
				switches[2]->down = false;
				break;
			case 3: case 4:
				switches[2]->up = false;
				switches[2]->down = true;
				break;
			case 5: case 6:
				switches[3]->up = true;
				switches[3]->down = false;
				break;
			case 7: case 8:
				switches[3]->up = false;
				switches[3]->down = true;
				break;
			default:
				printf("ERROR OUT EXT SWITCHES RIGHT: %d\n", d);
		}
	} else {
		switch (d) {
			case 1: case 2:
				switches[0]->up = true;
				switches[0]->down = false;
				break;
			case 3: case 4:
				switches[0]->up = false;
				switches[0]->down = true;
				break;
			case 5: case 6:
				switches[1]->up = true;
				switches[1]->down = false;
				break;
			case 7: case 8:
				switches[1]->up = false;
				switches[1]->down = true;
				break;
			default:
				printf("ERROR OUT EXT SWITCHES LEFT: %d\n", d);
			}
	}
}

/* Sets the value of internal switches for departing train */
void set_out_int_switches(int d, bool r) {
	if (r) {
		switch (d) {
			case 1:
				switches[8]->up = true;
				switches[8]->down = false;
				break;
			case 2:
				switches[8]->up = false;
				switches[8]->down = true;
				break;
			case 3:
				switches[9]->up = true;
				switches[9]->down = false;
				break;
			case 4:
				switches[9]->up = false;
				switches[9]->down = true;
				break;
			case 5:
				switches[10]->up = true;
				switches[10]->down = false;
				break;
			case 6:
				switches[10]->up = false;
				switches[10]->down = true;
				break;
			case 7:
				switches[11]->up = true;
				switches[11]->down = false;
				break;
			case 8:
				switches[11]->up = false;
				switches[11]->down = true;
				break;
			default:
				printf("ERROR OUT INT SWITCHES RIGHT: %d\n", d);
		}
	} else {
		switch (d) {
			case 1:
				switches[4]->up = true;
				switches[4]->down = false;
				break;
			case 2:
				switches[4]->up = false;
				switches[4]->down = true;
				break;
			case 3:
				switches[5]->up = true;
				switches[5]->down = false;
				break;
			case 4:
				switches[5]->up = false;
				switches[5]->down = true;
				break;
			case 5:
				switches[6]->up = true;
				switches[6]->down = false;
				break;
			case 6:
				switches[6]->up = false;
				switches[6]->down = true;
				break;
			case 7:
				switches[7]->up = true;
				switches[7]->down = false;
				break;
			case 8:
				switches[7]->up = false;
				switches[7]->down = true;
				break;
			default:
				printf("ERROR OUT INT SWITCHES LEFT: %d\n", d);
		}
	}
}

/* Sets the value of switches for departing train */
void set_out_switches(int d, bool r) {
	set_out_ext_switches(d, r);
	set_out_int_switches(d, r);
}

/* Sets the value of traffic lights for arriving train */
void set_in_lights(int d, bool r, bool g) {
	if (r) {
		lights[d - 1][0]->green = g;
	} else {
		lights[d - 1][1]->green = g;
	}
}

/* Sets the value of traffic lights for departing train */
void set_out_lights(int d, bool r, bool g) {
	if (r) {
		lights[d - 1][1]->green = g;
	} else {
		lights[d - 1][0]->green = g;
	}
}

/* Returns X position by dock */
int get_x_by_dock(int dock) {
	int ret;

	if (dock <= 4) {
		ret = UP_ENTRANCE_ABS;
	} else {
		ret = DOWN_ENTRANCE_ABS;
	}

	return ret;
}
