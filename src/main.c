#include <allegro.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "train.h"
#include "station.h"
#include "multimedia.h"
#include "rt_thread.h"

int main(int argc, char **argv){
  int val, ret;

  init_rt_thread_manager();
  init_train_manager();
  init_station();
  start_graphics();
  start_keyboard();

  while (!terminate) {
    for (int i = 0; i < N_MAX; i++){
      ret = load_train();

      if (ret == N_MAX) {
        break;
      }

      if (ret == -1) {
        printf("ABORT MAIN\n");
        return -1;
      }
    }

    while(!terminate & n_trains > 0);

    flush_trains();
  }

  close_train_schedule();
  stop_keyboard();
  stop_graphics();

  return 0;
}
