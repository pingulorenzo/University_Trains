#ifndef CONF_STATION_H
#define CONF_STATION_H 

#endif

#define R_MAX											25				//Number of rows
#define C_MAX											25				//Number of columns
#define DOCK_LEFT									10				//Index of left dock part
#define DOCK_RIGHT								14				//Index of right dock part
#define PLAT_1										2					//Index of first platform
#define PLAT_OFF									4					//Cells between platform couples
#define L_ENTRANCE_END						6					//Index of left entrance end
#define R_ENTRANCE_END						18				//Index of right entrance end
#define UP_ENTRANCE_R							6					//Index of up entrance row
#define DOWN_ENTRANCE_R						18				//Index of down entrance row
#define L_ENTRANCE_SWITCH_C				7					//Column of left entrance switches
#define R_ENTRANCE_SWITCH_C				17				//Column of right entrance switches
#define L_DOCK_SWITCH_C						9					//Column of left dock switches
#define R_DOCK_SWITCH_C						15				//Column of right dock switches
#define DOCK_SWITCH_1							3					//Row of first dock switch
#define SECTION_OFF								12				//Offset between same elements
#define DOCK_AREA_INDEX						12				// Index of docking area
/*----------------------------------------------------------------------------*/
#define L_ENTRANCE_ABS						0 - CELL_SIZE
#define R_ENTRANCE_ABS						WINDOW
#define UP_ENTRANCE_ABS						UP_ENTRANCE_R * CELL_SIZE
#define DOWN_ENTRANCE_ABS					DOWN_ENTRANCE_R * CELL_SIZE
#define DOCK_AREA_ABS							DOCK_AREA_INDEX * CELL_SIZE
#define RIGHT_DOCK_ABS						(DOCK_AREA_ABS) + CELL_SIZE
#define LEFT_DOCK_ABS							(DOCK_AREA_ABS) - CELL_SIZE
