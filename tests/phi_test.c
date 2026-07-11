#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int max_col = 32;
double phi = 0.0;
double one_phi_turn = 0.0;
double accumulator = 0.0;
double d_max_col = 0.0;
int times[32] = {0};

int nearest_integer(double x) {
		int low = (int)x, high = low + 1;
		double difflow = x - low;
		return (difflow <= 0.5) ? low : high;
}

unsigned int compute_next_column_index() {
		// Increment accumulator by one phith of a turn
		accumulator += one_phi_turn;
		// Greatest multiple of max_col less than or equal than accumulator
		int64_t greatest_multiple = (int64_t)(accumulator/d_max_col) * (int64_t)max_col;
		// Return the closest integer to their difference
		return nearest_integer(accumulator - (double)greatest_multiple); 
}

int main() {
	phi = (1.0 + sqrt(5.0))/2.0;
	one_phi_turn = (double)max_col / phi;
	d_max_col = (double)max_col;

	for(int i = 0; i < 1024; ++i) {
		int k = compute_next_column_index();
		times[k] += 1;
	}

	for(int i = 0; i < 32; ++i) {
		printf("%d ", times[i]);
	}
	return 0;
}



