/**
 * Very simple concurrent hash table implementation. Uses a collection of
 * concurrent lists under the hood.
 */

#include <stdatomic.h>
#include <limits.h>
#include <assert.h>

#ifndef HASH_KEY_T
#define HASH_KEY_T void *
#endif

#ifndef HASH_KEY_HASH_F
#define HASH_KEY_HASH_F(key) ((uint32_t)key)
#endif

#ifndef HASH_VAL_T
#define HASH_VAL_T void *
#endif

struct Hash_BacketValue {
	HASH_KEY_T key;
	HASH_VAL_T val;
};

static int
hash_backet_value_compare(struct Hash_BacketValue a, struct Hash_BacketValue b)
{
	return a.key - b.key;
}

#define LIST_UNIQUE 1
#define LIST_DATA_COMPARE_F(a, b) hash_backet_value_compare(a, b)
#define LIST_DATA_T struct Hash_BacketValue
#include "slfsl/list.h"

enum {
	HASH_ERROR_OOM,
	HASH_ERROR_ALREADY_EXISTS,
	HASH_ERROR_DOES_NOT_EXIST,
	HASH_ERROR_DROP_FAILED,
};

struct Hash {
	struct List *table;
	size_t backet_count;
};

/**
 * Initialize the hash table. Thread-unsafe.
 *
 * @param h             The hash table to initialize.
 * @param backet_count  The hash table backet count.
 * @return              HASH_ERROR_OOM on the memory allocation failure,
 *                      0 on success.
 */
int
hash_init(struct Hash *h, size_t backet_count)
{
	struct List *table = malloc(sizeof(*table) * backet_count);
	if (table == NULL) {
		return HASH_ERROR_OOM;
	}
	for (size_t i = 0; i < backet_count; i++) {
		list_init(&table[i]);
	}
	h->backet_count = backet_count;
	h->table = table;
	return 0;
}

/**
 * Put a value by the given key in the hash table if the key does not exist.
 * Thread-safe.
 *
 * @param h    The hash table to put the value into.
 * @param key  The key to put the value by.
 * @param val  The value to put.
 * @return     HASH_ERROR_ALREADY_EXISTS if the key exists already,
 *             0 on success.
 */
int
hash_insert(struct Hash *h, HASH_KEY_T key, HASH_VAL_T val)
{
	uint32_t hash = HASH_KEY_HASH_F(key) % h->backet_count;
	struct List *backet = &h->table[hash];
	struct Hash_BacketValue item = {key, val};
	if (list_insert(backet, item) == NULL) {
		return HASH_ERROR_ALREADY_EXISTS;
	}
	return 0;
}

#if 0
/**
 * 
 */
int
hash_delete(struct Hash *h, HASH_KEY_T key)
{
	uint32_t hash = HASH_KEY_HASH_F(key) % h->backet_count;
	struct List *backet = &h->table[hash];
	if (list_drop(backet, key) == 0) {
		return HASH_ERROR_DROP_FAILED;
	}
	return 0;
}

/**
 * Put a value by the given key in the hash table. Thread-safe.
 *
 * @param h    The hash table to put the value into.
 * @param key  The key to put the value by.
 * @param val  The value to put.
 * @return     0 on success.
 */
int
hash_replace(struct Hash *h, HASH_KEY_T key, HASH_VAL_T val)
{
	uint32_t hash = HASH_KEY_HASH_F(key) % h->backet_count;
	struct List *backet = h->table[hash];
	list_replace(backet, key);
	return 0;
}

/**
 * Get the value from the hash table by key. Thread-safe.
 *
 * @param h    The hash table to get the value from.
 * @param key  The key to get the value by.
 * @param val  The value destination.
 * @return     HASH_ERROR_DOES_NOT_EXIST if the key does not exist,
 *             0 on success.
 */
int
hash_get(struct Hash *h, HASH_KEY_T key, HASH_VAL_T *val)
{
	uint32_t hash = HASH_KEY_HASH_F(key) % h->backet_count;
	struct List *backet = h->table[hash];
	if (list_get(backet, key) == NULL) {
                return HASH_ERROR_DOES_NOT_EXIST;
        }
        return 0;
}
#endif

#undef HASH_VAL_T
#undef HASH_KEY_T
