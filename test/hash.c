#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define HASH_KEY_T uint32_t
#define HASH_KEY_HASH_F(key) key
#define HASH_VAL_T uint32_t
#include "../hash.h"

#define WRITER_INSERTS 1000000
#define WRITER_COUNT 16
#define INSERT_COUNT WRITER_INSERTS * WRITER_COUNT

static uint32_t rand_data_internal() {
	static uint32_t state = 1;
	return state = (uint64_t)state * 279470273u % 0xfffffffb;
}

static uint32_t random_data[INSERT_COUNT];

static void rand_data_init() {
	for (size_t i = 0; i < INSERT_COUNT; i++) {
		random_data[i] = rand_data_internal();
	}
}

static uint32_t rand_data() {
	static _Atomic uint32_t idx_next = 0;
	uint32_t idx = atomic_fetch_add_explicit(&idx_next, 1, memory_order_relaxed);
	if (idx >= INSERT_COUNT) {
		printf("INTERNAL ERROR: More writes than expected.\n");
		exit(1);
	}
	return random_data[idx];
}

static void *writer(void *data) {
	struct Hash *h = (struct Hash *)data;
	for (int i = 0; i < WRITER_INSERTS; i++) {
		uint32_t val = rand_data();
		uint32_t key = val;
		hash_insert(h, key, val);
	}
	return NULL;
}

static double
time_diff(struct timespec a, struct timespec b)
{
	size_t diff_sec = b.tv_sec - a.tv_sec;
	size_t diff_nsec = b.tv_nsec - a.tv_nsec;
	if (b.tv_nsec < a.tv_nsec) {
		diff_sec--;
		diff_nsec = 1000000000 + b.tv_nsec - a.tv_nsec;
	}
	return (double)diff_sec + (double)diff_nsec / 1000000000.0;
}

int main() {
	rand_data_init();
	struct Hash h = {};
	hash_init(&h, 1024);
	pthread_t twriter[WRITER_COUNT];
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (size_t i = 0; i < WRITER_COUNT; i++) {
		pthread_create(&twriter[i], NULL, writer, &h);
	}
	for (size_t i = 0; i < WRITER_COUNT; i++) {
		pthread_join(twriter[i], NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	double t = time_diff(begin, end);
	printf("RPS: %f\n", (double)INSERT_COUNT / t);
}
