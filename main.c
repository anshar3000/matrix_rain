#include "colors.h"
#include "globals.h"
#include "grid.h"
#include "list.h"
#include "pool_list.h"
#include "stream.h"
#include "timer.h"
#include <math.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

char *grid = NULL;
int *tail_size_lookup = NULL;
int max_row = 0;
int max_col = 0;
int timerfd = 0;
double phi = 0.0; // to be initilized
double one_phi_turn = 0.0;
double accumulator = 0.0;
double d_max_col = 0.0;

// static stream *stream_info = NULL;
stream* stream_info = NULL;

void handle_sigint(int sig) {
  if (grid)
    free(grid);
  if (stream_info)
    free(stream_info); // free(stream_info);
  if (tail_size_lookup)
    free(tail_size_lookup);
  if (timerfd > 0)
    close(timerfd);
  endwin();
  exit(0);
}

void setup_sigint_handler() {
  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
}

int nearest_integer(double x) {
  int low = (int)x, high = low + 1;
  double difflow = x - low;
  return (difflow <= 0.5) ? low : high;
}

unsigned int compute_next_column_index() {
  // Increment accumulator by one phith of a turn
  accumulator += one_phi_turn;
  // Greatest multiple of max_col less than or equal than accumulator
  int64_t greatest_multiple =
      (int64_t)(accumulator / d_max_col) * (int64_t)max_col;
  // Return the closest integer to their difference
  return nearest_integer(accumulator - (double)greatest_multiple);
}

// Make it rain, baby, rain
void matrix_rain(
    const unsigned int
        probability_of_new_stream, // max num of new streams per tick
    const unsigned int ms) {
  // Initialize stream info list
  stream_info = malloc(sizeof(stream) * (max_col + 1));
  if (stream_info == NULL) {
    fprintf(stderr, "matrix_rain -> malloc fail\n");
    return;
  }

  for (int i = 0; i < max_col + 1; ++i)
    stream_info[i] = (stream){.pos = -1};

  // Initialize timer
  init_timer(ms);

  // Main loop
  while (true) {
    // Generate up to 'probability_of_new_stream' streams
    for (int i = 0; i < (int)probability_of_new_stream; ++i) {
      int k = compute_next_column_index();

      if (stream_info[k].pos == -1) {
        stream new = draw_stream(new_stream(), k);
        stream_info[k] = new;
      }
    }

    // Draw and finalize each running stream
    for(int i = 0; i < max_col + 1; ++i) {
      stream s = stream_info[i];
      stream_info[i] = draw_stream(s, i);
    }

    // wait for next tick
    timer_wait(ms);
    refresh();
  }
  // theoretically unreachable...
}

struct args {
  int ms;                 // sleep time per iteration
};

struct args args_extract(int argc, char** argv) {
  int ms;
  
  if(argc == 1 || !(ms = atoi(argv[1]))) {
    return (struct args){100};
  }

  return (struct args){ms};
} 

int main(int argc, char** argv) 
{
  struct args args = args_extract(argc, argv);
  srand(time(NULL));
  setup_sigint_handler();
  initscr();

  // state initilization
  max_col = getmaxx(stdscr);
  max_row = getmaxy(stdscr);
  phi = (1.0 + sqrt(5.0)) / 2.0;
  one_phi_turn = (double)max_col / phi;
  d_max_col = (double)max_col;

  noecho();
  init_colors();
  init_tail_lookup();
  curs_set(0);
  init_char_grid();
  fill_grid();
  matrix_rain(1, args.ms);
  if (grid)
    free(grid);
  if (tail_size_lookup)
    free(tail_size_lookup);
  endwin();
  return 0;
}
