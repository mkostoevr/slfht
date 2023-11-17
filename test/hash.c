#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define HASH_KEY_T uint32_t
#define HASH_KEY_HASH_F(key) key
#define HASH_VAL_T uint32_t
#include "../hash.h"

#define WRITER_INSERTS 1000000
#define WRITER_COUNT 16
#define INSERT_COUNT WRITER_INSERTS * WRITER_COUNT

struct WriterState {
	struct Hash *h;
	size_t data_offset;
};

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

static void *writer(void *data) {
	struct Hash *h = ((struct WriterState *)data)->h;
	size_t data_offset = ((struct WriterState *)data)->data_offset;
	for (int i = data_offset; i < (data_offset + WRITER_INSERTS); i++) {
		uint32_t val = random_data[i];
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
	struct WriterState states[WRITER_COUNT];
	hash_init(&h, 1024);
	pthread_t twriter[WRITER_COUNT];
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (size_t i = 0; i < WRITER_COUNT; i++) {
		struct WriterState *state = &states[i];
		state->h = &h;
		state->data_offset = i * WRITER_INSERTS;
		pthread_create(&twriter[i], NULL, writer, &states[i]);
	}
	for (size_t i = 0; i < WRITER_COUNT; i++) {
		pthread_join(twriter[i], NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	double t = time_diff(begin, end);
	printf("RPS: %f\n", (double)INSERT_COUNT / t);
}
