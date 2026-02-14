#include <bits/time.h>
#include <bits/types/struct_itimerspec.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	// sleep
#include <string.h>
#include <signal.h>
#include <sys/timerfd.h>

#define MAX_STREAM_LENGTH 24
#define MIN_STREAM_LENGTH 8
#define MIN_STREAM_SPEED 1
#define MAX_STREAM_SPEED 4

#define BLACK_ATTRIBUTE 1
#define GREEN_ATTRIBUTE 2
#define WHITE_ATTRIBUTE 3

typedef struct {
	unsigned int color_pair;
	unsigned int attribute;
} attribute_pair;

typedef struct {
	int pos;
	int length;
	int speed;
} stream;

stream* stream_info = NULL;
char* grid = NULL;
int* tail_size_lookup = NULL;
int max_row = 0;
int max_col = 0;
int timerfd = 0;

const char matrix_rain_chars[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789"
"!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\ ";

// Set sighandler here for freeing grid on SIGINT
void handle_sigint(int sig) {
	if(grid) free(grid);
	if(stream_info) free(stream_info);
	if(tail_size_lookup) free(tail_size_lookup);
	if(timerfd > 0) close(timerfd);
  endwin();
	exit(0);
}

// Listening sigint
void setup_sigint_handler() {
	struct sigaction sa;
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
}

// Get color and attribute based on distance from head of stream
attribute_pair get_attr_pair_given_dist(int dist_from_head, int length)
{
	// int tail_size = 3 + (length - MIN_STREAM_LENGTH + 1)/(MAX_STREAM_LENGTH - MIN_STREAM_LENGTH + 1)*5;
	int tail_size = tail_size_lookup[length - MIN_STREAM_LENGTH];

	if(dist_from_head == 0) {
		return (attribute_pair) {COLOR_PAIR(WHITE_ATTRIBUTE), A_BOLD};	
	} else if(dist_from_head == 1) {
		return (attribute_pair) {COLOR_PAIR(WHITE_ATTRIBUTE), A_NORMAL};
	} else if(dist_from_head <= length - tail_size) {
		return (attribute_pair) {COLOR_PAIR(GREEN_ATTRIBUTE), A_NORMAL};
	} else {
		return (attribute_pair) {COLOR_PAIR(GREEN_ATTRIBUTE), A_DIM};	
	}
}

// Set character at (row, col) with color and attribute
void set_color_at(int row, int col, char c, unsigned int color_pair, unsigned int attribute) {
	chtype ch = (chtype) c | color_pair | attribute;
	mvwaddch(stdscr, row, col, ch);
}

// Get a random character from the matrix_rain_chars array
char get_random_char() {
	int idx = rand() % (sizeof(matrix_rain_chars) -1);
	return matrix_rain_chars[idx];
}

// Initialize a grid of random characters
void init_char_grid() {
	grid = malloc(max_col * max_row);
	for(int i = 0; i < max_row; ++i) {
		for(int k = 0; k < max_col; ++k) {
			grid[max_col*i + k] = get_random_char();
		}
	}
}

// Get character from grid at (row, col)
char get_char_from_grid(int row, int col) {
	return grid[max_col*row + col];
}

// Initialize color pairs
void init_colors() {
	start_color();
	init_pair(BLACK_ATTRIBUTE, COLOR_BLACK, COLOR_BLACK);
	init_pair(GREEN_ATTRIBUTE, COLOR_GREEN, COLOR_BLACK);
	init_pair(WHITE_ATTRIBUTE, COLOR_WHITE, COLOR_BLACK);
}

// Initialize tail size lookup (saving one division)
void init_tail_lookup() {
	tail_size_lookup = malloc(sizeof(int) * (MAX_STREAM_LENGTH - MIN_STREAM_LENGTH));
	for(int i = 0; i < (MAX_STREAM_LENGTH - MIN_STREAM_LENGTH) + 1; ++i){
	 tail_size_lookup[i] = 2*(MIN_STREAM_LENGTH + i) / 5;
	}
}

// Initialize timer with ms
void init_timer(const unsigned int ms) {
  struct itimerspec tv = (struct itimerspec){0};
  timerfd = timerfd_create(CLOCK_MONOTONIC, 0); /* Global variable assignment */

  // if timerfd is invalid, we'll just use usleep instead
  if(timerfd < 0) {
    goto _init_timer_error;
  }
  
  // Set timer values
  tv.it_value.tv_sec = 0;
  tv.it_value.tv_nsec = ms * 100 * 100 * 100;
  tv.it_interval.tv_sec = 0;
  tv.it_interval.tv_nsec = ms * 100 * 100 * 100;

  timerfd_settime(timerfd, 0, &tv, NULL);
  return;

_init_timer_error:
  setbuf(stderr, NULL);
  fprintf(stderr, "WARNING: timerfd error. Using usleep for timer interval management\n");
}

// Draw stream at column col_index, return updated stream info
stream draw_stream(stream s, int col_index)
{
	int stream_start_pos = s.pos;
	int length = s.length;
	int tail_size = s.speed;

	// if stream has ended
	if (stream_start_pos < 0) {
		return (stream) {-1, 0, 0};
	}

	int row_index = (max_row - 1 > stream_start_pos) ? stream_start_pos : max_row - 1;
	int dist_from_head = stream_start_pos - row_index;

	if(dist_from_head < s.length)
	{
		// draw head and body
		for(;row_index >= 0 && dist_from_head <= s.length; dist_from_head++, row_index--)
		{
			// update character at this position
			char c = get_char_from_grid(row_index, col_index);

			// get color and attribute based on distance from head
			attribute_pair p = get_attr_pair_given_dist(dist_from_head, s.length);
			
			// set character with color and attribute
			set_color_at(row_index, col_index, c, p.color_pair, p.attribute);
		}

		// update stream position
		s = (stream) {stream_start_pos + s.speed, s.length, s.speed};	
	} else {
		// stream ended
		s = (stream) {-1, 0, 0};
	}

	// clear tail
	while(row_index >= 0 && tail_size >= 0) {	
		char c = get_char_from_grid(row_index, col_index);
		set_color_at(row_index, col_index, c, COLOR_PAIR(BLACK_ATTRIBUTE), A_NORMAL);
		row_index--; tail_size--;
	}

	return s;
}

// Make it rain, baby, rain
void matrix_rain(const int probability_of_new_stream,
                 const unsigned int ms) 
{
	// marks the postion of the head of rain drop
	stream_info = malloc(sizeof(stream) * max_col);

  // Initialize timer
  init_timer(ms);

	for(int i = 0; i < max_col; ++i) {
		stream_info[i] = (stream) {-1, 0, 0};
	}

	while(true) 
	{
		for(int col_index = 0; col_index < max_col; ++col_index)
		{
      // collect stream info at col_index
			stream s = stream_info[col_index];

      // if no stream at column
			if(s.pos == -1) 
      {
				// start a new stream with some probability
				unsigned int rand_val = rand();
	      bool has_stream_left = (col_index != max_col - 1) && stream_info[col_index + 1].pos != -1;
     		bool has_stream_right = has_stream_left || ((col_index != 0) && stream_info[col_index - 1].pos != -1);

				if((rand_val % 1024) > (1024 - probability_of_new_stream) &&
            !has_stream_right && !has_stream_left)
        {
					s.pos = 0;
					s.length = MIN_STREAM_LENGTH + (rand_val % (MAX_STREAM_LENGTH - MIN_STREAM_LENGTH));
					s.speed = MIN_STREAM_SPEED + (rand_val % (MAX_STREAM_SPEED - MIN_STREAM_SPEED));
				}
			}

			// draw stream and update stream info
			stream_info[col_index] = draw_stream(s, col_index);
		}
		
		if(timerfd < 0) {
        usleep(ms * 1000);
    } else {
        read(timerfd, NULL, sizeof(uint64_t));
    }
		
    refresh();
	} 

  if(timerfd > 0) close(timerfd);
	free(stream_info);
}

// Fill the entire grid with characters from grid array and BLACK_ATTRIBUTE
void fill_grid()
{
	attron(COLOR_PAIR(BLACK_ATTRIBUTE));

	for(int i = 0; i < max_row; ++i) {
		for(int k = 0; k < max_col; ++k) {
			char c = get_char_from_grid(i, k);
			mvwaddch(stdscr, i, k, c);
		}
	}
	
	attroff(COLOR_PAIR(BLACK_ATTRIBUTE));
	refresh();
}

int main() {
	srand(time(NULL));	// seeding PRNG
	setup_sigint_handler();
	initscr();
	max_col = getmaxx(stdscr);
	max_row = getmaxy(stdscr);
	noecho();
	init_colors();
	init_tail_lookup();
	curs_set(0);	// hide cursor
	init_char_grid();
	fill_grid();
	matrix_rain(16, 100);	
	if(grid) free(grid);
	if(tail_size_lookup) free(tail_size_lookup);
	endwin();
	return 0;
}
