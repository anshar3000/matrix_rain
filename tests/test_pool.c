#include "../pool_list.h"
#include "list.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

const int n = 1024;
int phase = 0;
linked_list_t ll = NULL;

void handle_sigsev(int sig) {
	fprintf(stderr, "SIGSEGV at phase %d\n", phase);
	exit(1);
}

void setup_sigsev_handler() {
	struct sigaction sa;
	sa.sa_handler = handle_sigsev;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGSEGV, &sa, NULL);
}

bool remove_even(void* ctx, void* data) {
	(void)ctx;
	long i = (long)data;
	return i % 2 == 0;
}

void print_all(void* ctx, void* data) {
	(void)ctx;
	printf("%ld\n", (long)data);
}

int main() {
	setup_sigsev_handler();
	
	// Phase 0 - create list
	ll = pool_list_create(n);	
	phase += 1;
	
	// Phase 1 - add elements to list
	for(int i = 0; i < n-1; ++i) {
		linked_list_push_back(ll, (void *)(size_t)i);
	}
	phase += 1;
	
	// Phase 2 - manually remove elements from list
	for(int i = 0; i < n-1; ++i) {
		ll_maybe_t x = linked_list_pop_back(ll);
		printf("%d - %ld\n", i, (long)x.value); 
	}
	phase += 1;

	// Phase 3 - readd elements to list
	for(int i = 0; i < n-1; ++i) {
		linked_list_push_back(ll, (void *)(size_t)i);
	}
	phase += 1;

	// Phase 4 - selectively remove even elements
	linked_list_remove_if(ll, remove_even, NULL);
	phase += 1;

	// Phase 5 - print all elements
	linked_list_for_each(ll, print_all, NULL);
	phase += 1;
	
	// Free memory
	printf("All phases passed\n");
	pool_list_free(ll);
	return 0;
}
