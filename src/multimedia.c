#include "multimedia.h"
#include "train.h"
#include "station.h"

#define TRACK_UP_DOWN_PATH      "./img/track_vertical.bmp"
#define TRACK_LEFT_RIGHT_PATH   "./img/track.bmp"
#define TRACK_UP_LEFT_PATH      "./img/track_up_left.bmp"
#define TRACK_UP_RIGHT_PATH     "./img/track_up_right.bmp"
#define TRACK_DOWN_LEFT_PATH    "./img/track_down_left.bmp"
#define TRACK_DOWN_RIGHT_PATH   "./img/track_down_right.bmp"
#define DOCK_TRACK_PATH         "./img/dock_track.bmp"
#define TILE_PATH               "./img/tile.bmp"
#define LOCO_PATH               "./img/loco.bmp"
#define WAGON_PATH              "./img/wagon.bmp"
#define RED_LIGHT_PATH          "./img/red_light.bmp"
#define GREEN_LIGHT_PATH        "./img/green_light.bmp"

unsigned int graphics_tid, keyboard_tid;     // threads IDs
bool terminate;

BITMAP *screen_buffer;
BITMAP *tile;
BITMAP *loco;
BITMAP *wagon_b;
BITMAP *loco_hor_right;
BITMAP *loco_hor_left;
BITMAP *loco_ver_up;
BITMAP *loco_ver_down;
BITMAP *wagon_hor_right;
BITMAP *wagon_hor_left;
BITMAP *wagon_ver_up;
BITMAP *wagon_ver_down;
BITMAP *track;
BITMAP *track_vertical;
BITMAP *track_up_left;
BITMAP *track_up_right;
BITMAP *track_down_left;
BITMAP *track_down_right;
BITMAP *dock_track;
BITMAP *green_light;
BITMAP *red_light;
BITMAP *cell_bmp[R_MAX][C_MAX];

/* ======================================
*  ============== GRAPHICS ==============
*  ====================================== */

/* Start graphics and graphic thread */
void start_graphics(void) {
  int ret;

  allegro_init();

  set_color_depth(24);
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, WINDOW, WINDOW, 0, 0);

  screen_buffer = create_bitmap(WINDOW, WINDOW);
  tile = load_bitmap(TILE_PATH, NULL);
  loco = load_bitmap(LOCO_PATH, NULL);
  wagon_b = load_bitmap(WAGON_PATH, NULL);
  track = load_bitmap(TRACK_LEFT_RIGHT_PATH, NULL);
  track_vertical = load_bitmap(TRACK_UP_DOWN_PATH, NULL);
  track_down_right = load_bitmap(TRACK_DOWN_RIGHT_PATH, NULL);
  track_down_left = load_bitmap(TRACK_DOWN_LEFT_PATH, NULL);
  track_up_right = load_bitmap(TRACK_UP_RIGHT_PATH, NULL);
  track_up_left = load_bitmap(TRACK_UP_LEFT_PATH, NULL);
  dock_track = load_bitmap(DOCK_TRACK_PATH, NULL);
  green_light = load_bitmap(GREEN_LIGHT_PATH, NULL);
  red_light = load_bitmap(RED_LIGHT_PATH, NULL);

  for (int j = 0; j < C_MAX; j++){
    for (int i = 0; i < R_MAX; i++){
      cell_bmp[i][j] = create_bitmap(CELL_SIZE, CELL_SIZE);
    }
  }

  terminate = false;

  lw_allocate(&loco_ver_up);
  lw_allocate(&loco_ver_down);
  lw_allocate(&loco_hor_left);
  lw_allocate(&loco_hor_right);
  lw_allocate(&wagon_ver_up);
  lw_allocate(&wagon_ver_down);
  lw_allocate(&wagon_hor_left);
  lw_allocate(&wagon_hor_right);

  masked_stretch_blit(loco, loco_ver_up, 0, 0, loco->w, loco->h, 0, 0, CELL_SIZE, CELL_SIZE);
  rotate_sprite(loco_hor_right, loco_ver_up, 0, 0, itofix(64));
  rotate_sprite(loco_ver_down, loco_ver_up, 0, 0, itofix(128));
  rotate_sprite(loco_hor_left, loco_ver_up, 0, 0, itofix(192));
  masked_stretch_blit(wagon_b, wagon_ver_up, 0, 0, wagon_b->w, wagon_b->h, 0, 0, CELL_SIZE, CELL_SIZE);
  rotate_sprite(wagon_hor_right, wagon_ver_up, 0, 0, itofix(64));
  rotate_sprite(wagon_ver_down, wagon_ver_up, 0, 0, itofix(128));
  rotate_sprite(wagon_hor_left, wagon_ver_up, 0, 0, itofix(192));

  install_keyboard();

  ret = start_thread(graphic_behaviour, NULL, SCHED_FIFO,
          PRD_GRAPHICS, DL_GRAPHICS, PRIO_GRAPHICS);
  if (ret < 0) {
      printf("ERROR GRAPHIC THREAD\n");
      return;
  } else {
      graphics_tid = ret;
  }
}

/* Allocates bitmap for locomotive or wagon */
void lw_allocate(BITMAP **lw_act){
  *lw_act = create_bitmap(CELL_SIZE, CELL_SIZE);
  clear_to_color(*lw_act, makecol(255, 0, 255));
}

/* Gracefully stops graphic thread */
void stop_graphics(void) {
  stop_thread(graphics_tid);
  allegro_exit();
}

/* Function to be called by graphic thread */
void *graphic_behaviour() {
  time_g = (time_g + TIME_STEP) % (TIME_MAX);
  //printf("Time: %d\n", time_g);

  update_all_cells();
  draw_all_cells();

  //LAST INSTRUCTIONS TO CALL
  draw_all_trains();

  //DOUBLE BUFFERING TECHNIQUE
  blit(screen_buffer, screen, 0, 0, 0, 0, WINDOW, WINDOW);
  clear_bitmap(screen_buffer);
}

void *keyboard_behaviour(void *arg) {
  int val;

  if (keypressed()){
    val = readkey();
    val = val >> 8;
    if (val == KEY_SPACE) {
      printf("Exit from key press\n");
      terminate = true;
    }
    if (val == KEY_RIGHT) {
      printf("Spawn train heading right\n");
      load_my_train(true);
    }
    if (val == KEY_LEFT) {
      printf("Spawn train heading left\n");
      load_my_train(false);
    }
  }
}

int start_keyboard() {
  int ret;

  ret = start_thread(keyboard_behaviour, NULL, SCHED_FIFO,
                PRD_KEYBOARD, DL_KEYBOARD, PRIO_KEYBOARD);

  if (ret < 0) {
      printf("ERROR GRAPHIC THREAD\n");
      return -1;
  } else {
      keyboard_tid = ret;
  }

  return ret;
}

void stop_keyboard() {
  stop_thread(graphics_tid);
}

/* Refreshes all cells */
void draw_all_cells(void) {
  for (int j = 0; j < C_MAX; j++){
    for (int i = 0; i < R_MAX; i++){
      draw_sprite(screen_buffer, cell_bmp[i][j], j * CELL_SIZE, i * CELL_SIZE);
    }
  }
}

/* Updates cells position */
void update_all_cells(void) {
  for (int j = 0; j < C_MAX; j++){
    for (int i = 0; i < R_MAX; i++){
      masked_stretch_blit(tile, cell_bmp[i][j], 0, 0, tile->w, tile->h, 0, 0, CELL_SIZE, CELL_SIZE);
      switch (cells[i][j].content) {
        case TILE:
          break;
        case TRACK:
          if (cells[i][j].up) {
            if (cells[i][j].left) {
              masked_stretch_blit(track_up_left, cell_bmp[i][j], 0, 0, track_up_left->w, track_up_left->h, 0, 0, CELL_SIZE, CELL_SIZE);
            } else if (cells[i][j].right) {
              masked_stretch_blit(track_up_right, cell_bmp[i][j], 0, 0, track_up_right->w, track_up_right->h, 0, 0, CELL_SIZE, CELL_SIZE);
            } else {
              masked_stretch_blit(track_vertical, cell_bmp[i][j], 0, 0, track_vertical->w, track_vertical->h, 0, 0, CELL_SIZE, CELL_SIZE);
            }
          } else if (cells[i][j].down) {
            if (cells[i][j].left) {
              masked_stretch_blit(track_down_left, cell_bmp[i][j], 0, 0, track_down_left->w, track_down_left->h, 0, 0, CELL_SIZE, CELL_SIZE);
            } else {
              masked_stretch_blit(track_down_right, cell_bmp[i][j], 0, 0, track_down_right->w, track_down_right->h, 0, 0, CELL_SIZE, CELL_SIZE);
            }
          } else {
            masked_stretch_blit(track, cell_bmp[i][j], 0, 0, track->w, track->h, 0, 0, CELL_SIZE, CELL_SIZE);
          }
          break;
        case DOCK_TRACK:
          masked_stretch_blit(dock_track, cell_bmp[i][j], 0, 0, dock_track->w, dock_track->h, 0, 0, CELL_SIZE, CELL_SIZE);
          break;
        case SEM:
          if (cells[i][j].green) {
            masked_stretch_blit(green_light, cell_bmp[i][j], 0, 0, green_light->w, green_light->h, 0, 0, CELL_SIZE, CELL_SIZE);
          } else {
            masked_stretch_blit(red_light, cell_bmp[i][j], 0, 0, red_light->w, red_light->h, 0, 0, CELL_SIZE, CELL_SIZE);
          }
          break;
        default:
          printf("ERROR UPDATE CELL\n");
      }
    }
  }
}

/* Refreshes trains */
void draw_all_trains(void) {
  BITMAP *loco_dir, *wag[2];

  for (int i = 0; i < N_MAX; i++) {
    loco_dir = NULL;
    wag[0] = NULL;
    wag[1] = NULL;
    if (trains[i].alive && trains[i].status != ARRIVING) {
      switch (trains[i].pos.next) {
        case UP_N:
          loco_dir = loco_ver_up;
          break;
        case RIGHT_N:
          loco_dir = loco_hor_right;
          break;
        case DOWN_N:
          loco_dir = loco_ver_down;
          break;
        case LEFT_N:
          loco_dir = loco_hor_left;
          break;
        default:
          printf("ERROR DRAWING TRAINS\n");
      }

      for (int j = 1; j >= 0; j--){
        switch (trains[i].wagons[j].pos.next) {
          case UP_N:
            wag[j] = wagon_ver_up;
            break;
          case RIGHT_N:
            wag[j] = wagon_hor_right;
            break;
          case DOWN_N:
            wag[j] = wagon_ver_down;
            break;
          case LEFT_N:
            wag[j] = wagon_hor_left;
            break;
          default:
            printf("ERROR DRAWING WAGONS\n");
        }

        if (wag[j] != NULL) {
          draw_sprite(screen_buffer, wag[j], trains[i].wagons[j].pos.y, trains[i].wagons[j].pos.x);
        }
      }
    }
    if (loco_dir != NULL) {
      draw_sprite(screen_buffer, loco_dir, trains[i].pos.y, trains[i].pos.x);
    }
  }
}
