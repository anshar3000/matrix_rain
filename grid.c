#include "grid.h"
#include "globals.h"
#include <ncurses.h>
#include <stdlib.h>
#include "colors.h"

static const char matrix_rain_chars[] =
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789"
"!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\ ";

static char get_random_char() {
	int idx = rand() % (sizeof(matrix_rain_chars) -1);
	return matrix_rain_chars[idx];
}

void init_char_grid() {
	grid = malloc(max_col * max_row);
	for(int i = 0; i < max_row; ++i)
		for(int k = 0; k < max_col; ++k)
			grid[max_col*i + k] = get_random_char();
}

char get_char_from_grid(int row, int col) {
	return grid[max_col*row + col];
}

void fill_grid() {
	attron(COLOR_PAIR(BLACK_ATTRIBUTE));
	for(int i = 0; i < max_row; ++i)
		for(int k = 0; k < max_col; ++k)
			mvwaddch(stdscr, i, k, get_char_from_grid(i, k));
	attroff(COLOR_PAIR(BLACK_ATTRIBUTE));
	refresh();
}
