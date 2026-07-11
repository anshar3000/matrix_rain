#include "stream.h"
#include "colors.h"
#include "grid.h"
#include "globals.h"
#include <ncurses.h>
#include <stdlib.h>

void init_tail_lookup() {
	tail_size_lookup = malloc(sizeof(int) * (MAX_STREAM_LENGTH - MIN_STREAM_LENGTH + 1));
	for(int i = 0; i < (MAX_STREAM_LENGTH - MIN_STREAM_LENGTH) + 1; ++i)
		tail_size_lookup[i] = 2*(MIN_STREAM_LENGTH + i) / 5;
}

static void set_color_at(int row, int col, char c, unsigned int color_pair, unsigned int attribute) {
	chtype ch = (chtype) c | color_pair | attribute;
	mvwaddch(stdscr, row, col, ch);
}

stream draw_stream(stream s, int col_index)
{
	int stream_start_pos = s.pos;
	int tail_size        = s.speed;

	if (stream_start_pos < 0)
		return (stream) {-1, 0, 0};

	int row_index      = (max_row - 1 > stream_start_pos) ? stream_start_pos : max_row - 1;
	int dist_from_head = stream_start_pos - row_index;

	if(dist_from_head < s.length)
	{
		for(;row_index >= 0 && dist_from_head <= s.length; dist_from_head++, row_index--)
		{
			char c = get_char_from_grid(row_index, col_index);
			attribute_pair p = get_attr_pair_given_dist(dist_from_head, s.length);
			set_color_at(row_index, col_index, c, p.color_pair, p.attribute);
		}
		s = (stream) {stream_start_pos + s.speed, s.length, s.speed};
	} else {
		s = (stream) {-1, 0, 0};
	}

	while(row_index >= 0 && tail_size >= 0) {
		char c = get_char_from_grid(row_index, col_index);
		set_color_at(row_index, col_index, c, COLOR_PAIR(BLACK_ATTRIBUTE), A_NORMAL);
		row_index--; tail_size--;
	}

	return s;
}

stream new_stream() {
		stream s;
		unsigned int rand_val = rand();
 		s.pos    = 0;
 		s.length = (short int)(MIN_STREAM_LENGTH + (rand_val % (MAX_STREAM_LENGTH - MIN_STREAM_LENGTH)));
 		s.speed  = (short int)(MIN_STREAM_SPEED  + (rand_val % (MAX_STREAM_SPEED - MIN_STREAM_SPEED)));
		return s;
}

bool stream_is_finalized(void* ctx, void* data) {
	stream *is_set = (stream *)ctx;
	int k = (int)data;
	return (is_set[k] = draw_stream(is_set[k], k)).pos == -1;
}
