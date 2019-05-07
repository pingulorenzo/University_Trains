#include "station.h"

cell cells[R_MAX][C_MAX];						// matrix of cell descriptors
cell *switches[3 * N_SECTION];			// array of pointer to cells containing switches
cell *lights[N_PLATFORM][2];				// matrix of pointer to cells containing lights

/* Builds station */
void init_station(void) {
	init_docks();
	init_docks_links();
	init_entrances();
	init_switches();
	init_mid_tracks();
	init_lights();
	assign_switches();
}

/* Builds docks' position on station */
void init_docks() {
	for (int j = DOCK_LEFT; j <= DOCK_RIGHT; j++) {
		for (int i = 0; i < N_PLATFORM / 2; i++) {
			int row_act;

			//FIRST DOCK LINE
			row_act = PLAT_1 + i * ((PLAT_OFF - 1) * 2);
			cells[row_act][j].content = DOCK_TRACK;
			cells[row_act][j].left = true;
			cells[row_act][j].right = true;

			//SECOND DOCK LINE
			row_act += 2;
			cells[row_act][j].content = DOCK_TRACK;
			cells[row_act][j].left = true;
			cells[row_act][j].right = true;
		}
	}
}

/* Builds docks' links position on station */
void init_docks_links() {
	for (int i = 0; i < N_PLATFORM / 2; i++) {
		int row_act, col_act;

		//FIRST DOCK LINE
		row_act = PLAT_1 + i * ((PLAT_OFF - 1) * 2);
		col_act = DOCK_LEFT - 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].down = true;
		cells[row_act][col_act].right = true;
		col_act = DOCK_RIGHT + 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].down = true;

		//SECOND DOCK LINE
		row_act += 2; 
		col_act = DOCK_LEFT - 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].up = true;
		cells[row_act][col_act].right = true;
		col_act = DOCK_RIGHT + 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].up = true;
	}
}

/* Builds entrances on station */
void init_entrances() {
	for (int j = 0; j <= L_ENTRANCE_END; j++) {
		int row_act, col_act;

		//FIRST ENTRANCE LINE
		row_act = UP_ENTRANCE_R;
		col_act = j;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].right = true;

		//SECOND ENTRANCE LINE
		row_act = DOWN_ENTRANCE_R;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].right = true;

		//THIRD ENTRANCE LINE
		row_act = UP_ENTRANCE_R;
		col_act = C_MAX - 1 - j;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].right = true;

		//FOURTH ENTRANCE LINE
		row_act = DOWN_ENTRANCE_R;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].right = true;
	}
}

/* Builds switches on station */
void init_switches() {
	int row_act, col_act;

	for (int i = 0; i < N_PLATFORM / 2; i++) {

		// MID DOCK LINE LEFT
		row_act = DOCK_SWITCH_1 + i * ((PLAT_OFF - 1) * 2);
		col_act = DOCK_LEFT - 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].up = true;

		//MID DOCK LINE RIGHT
		col_act = DOCK_RIGHT + 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].right = true;
		cells[row_act][col_act].up = true;
	}

	//FIRST ENTRANCE LINE
	row_act = UP_ENTRANCE_R;
	col_act = L_ENTRANCE_SWITCH_C;
	cells[row_act][col_act].content = TRACK;
	cells[row_act][col_act].left = true;
	cells[row_act][col_act].up = true;

	//SECOND ENTRANCE LINE
	row_act = DOWN_ENTRANCE_R;
	cells[row_act][col_act].content = TRACK;
	cells[row_act][col_act].left = true;
	cells[row_act][col_act].up = true;

	//THIRD ENTRANCE LINE
	row_act = UP_ENTRANCE_R;
	col_act = R_ENTRANCE_SWITCH_C;
	cells[row_act][col_act].content = TRACK;
	cells[row_act][col_act].right = true;
	cells[row_act][col_act].up = true;

	//FOURTH ENTRANCE LINE
	row_act = DOWN_ENTRANCE_R;
	cells[row_act][col_act].content = TRACK;
	cells[row_act][col_act].right = true;
	cells[row_act][col_act].up = true;
}

/* Builds mid tracks on station */
void init_mid_tracks() {
int row_act, col_act;

	for (int i = 1; i < 3; i++) {
		for (int j = 0; j < 2; j++) {

			//LEFT PART
			row_act = UP_ENTRANCE_R + j * SECTION_OFF;
			col_act = L_ENTRANCE_SWITCH_C;
			cells[row_act + i][col_act].content = TRACK;
			cells[row_act + i][col_act].down = true;
			cells[row_act + i][col_act].up = true;
			cells[row_act - i][col_act].content = TRACK;
			cells[row_act - i][col_act].down = true;
			cells[row_act - i][col_act].up = true;

			//RIGHT PART
			col_act = R_ENTRANCE_SWITCH_C;
			cells[row_act + i][col_act].content = TRACK;
			cells[row_act + i][col_act].down = true;
			cells[row_act + i][col_act].up = true;
			cells[row_act - i][col_act].content = TRACK;
			cells[row_act - i][col_act].down = true;
			cells[row_act - i][col_act].up = true;
		}
	}

	//LEFT PART
	for (int j = 0; j < 2; j++) {
		int i = 3;

		//LEFT PART
		row_act = UP_ENTRANCE_R + j * SECTION_OFF;
		col_act = L_ENTRANCE_SWITCH_C;
		cells[row_act + i][col_act].content = TRACK;
		cells[row_act + i][col_act].up = true;
		cells[row_act + i][col_act].right = true;
		cells[row_act - i][col_act].content = TRACK;
		cells[row_act - i][col_act].down = true;
		cells[row_act - i][col_act].right = true;

		//RIGHT PART
		col_act = R_ENTRANCE_SWITCH_C;
		cells[row_act + i][col_act].content = TRACK;
		cells[row_act + i][col_act].up = true;
		cells[row_act + i][col_act].left = true;
		cells[row_act - i][col_act].content = TRACK;
		cells[row_act - i][col_act].down = true;
		cells[row_act - i][col_act].left = true;
	}

	for (int i = 0; i < N_PLATFORM / 2; i++) {

		//LEFT PART
		row_act = DOCK_SWITCH_1 + i * ((PLAT_OFF - 1) * 2);
		col_act = L_DOCK_SWITCH_C - 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].left = true;
		cells[row_act][col_act].right = true;

		//RIGHT PART
		col_act = R_DOCK_SWITCH_C + 1;
		cells[row_act][col_act].content = TRACK;
		cells[row_act][col_act].right = true;
		cells[row_act][col_act].left = true;
	}
}

/* Builds lights on station */
void init_lights(void) {
	for (int i = 0; i < N_PLATFORM / 2; i++) {
		int row_act, col_act;

		//FIRST DOCK LINE
		row_act = (PLAT_1 - 1) + i * ((PLAT_OFF - 1) * 2);
		col_act = DOCK_LEFT - 1;
		cells[row_act][col_act].content = SEM;
		cells[row_act][col_act].green = false;
		lights[i * 2][0] = &cells[row_act][col_act];
		col_act = DOCK_RIGHT + 1;
		cells[row_act][col_act].content = SEM;
		cells[row_act][col_act].green = false;
		lights[i * 2][1] = &cells[row_act][col_act];

		//SECOND DOCK LINE
		row_act += 4;
		col_act = DOCK_LEFT - 1;
		cells[row_act][col_act].content = SEM;
		cells[row_act][col_act].green = false;
		lights[i * 2 + 1][0] = &cells[row_act][col_act];
		col_act = DOCK_RIGHT + 1;
		cells[row_act][col_act].content = SEM;
		cells[row_act][col_act].green = false;
		lights[i * 2 + 1][1] = &cells[row_act][col_act];
	}
}

/* Assigns switches pointers to switches[] array */
void assign_switches(void) {
	switches[0] = &cells[6][7];
	switches[1] = &cells[18][7];
	switches[2] = &cells[6][17];
	switches[3] = &cells[18][17];
	switches[4] = &cells[3][9];
	switches[5] = &cells[9][9];
	switches[6] = &cells[15][9];
	switches[7] = &cells[21][9];
	switches[8] = &cells[3][15];
	switches[9] = &cells[9][15];
	switches[10] = &cells[15][15];
	switches[11] = &cells[21][15];
}
