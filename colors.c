#include "colors.h"
#include "globals.h"
#include <ncurses.h>

void init_colors() {
	start_color();
	init_pair(BLACK_ATTRIBUTE, COLOR_BLACK, COLOR_BLACK);
	init_pair(GREEN_ATTRIBUTE, COLOR_GREEN, COLOR_BLACK);
	init_pair(WHITE_ATTRIBUTE, COLOR_WHITE, COLOR_BLACK);
}

attribute_pair get_attr_pair_given_dist(int dist_from_head, int length) {
	int tail_size = tail_size_lookup[length - MIN_STREAM_LENGTH];

	switch(dist_from_head) {
		case 0:
			return (attribute_pair) {COLOR_PAIR(WHITE_ATTRIBUTE), A_BOLD};
		case 1:
			return (attribute_pair) {COLOR_PAIR(WHITE_ATTRIBUTE), A_NORMAL};
		default:
			return (dist_from_head <= length - tail_size) ?
				(attribute_pair) {COLOR_PAIR(GREEN_ATTRIBUTE), A_NORMAL} :
				(attribute_pair) {COLOR_PAIR(GREEN_ATTRIBUTE), A_DIM};
	}
}
