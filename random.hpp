#pragma once

#include <stdlib.h> 
#include <time.h>

static unsigned rand_in_range(unsigned start, unsigned end) {
	unsigned range = end - start;
	if (range == 0) {
		return start;
	}
	range += 1;
	return (rand() % range) + start;
}

static bool rand_bool(void) {
	return ((rand() % 2) == 0) ? true : false;
}

static bool rand_bool_dist(unsigned range) {
	return (rand_in_range(0, range) == 0) ? true : false;
}

static void rand_set_time_seed(void) {
	srand(static_cast<unsigned int>(time(NULL)));
}